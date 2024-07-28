#include "YUME/yumepch.h"
#include "vulkan_context.h"

#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"

// Lib
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace YUME
{
#ifdef YM_DEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity,
		VkDebugUtilsMessageTypeFlagsEXT p_Type,
		const VkDebugUtilsMessengerCallbackDataEXT* p_CallbackData,
		void* p_UserData)
	{
		// TODO: May be refactored this

		YM_CORE_TRACE("Debug Callback:")
		YM_CORE_TRACE("{}", p_CallbackData->pMessage)
		YM_CORE_TRACE("\t Severity: {}", Utils::GetMessageSeverityStr(p_Severity))
		YM_CORE_TRACE("\t Type: {}", Utils::GetMessageType(p_Type))
		YM_CORE_TRACE("\t Objects: ")

		for (uint32_t i = 0; i < p_CallbackData->objectCount; i++) {
			YM_CORE_TRACE("\t\t {}", p_CallbackData->pObjects[i].objectHandle)
		}

		std::cout << "\n\n";

		return VK_FALSE;  // The calling function should not be aborted
	}

	static VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo{};
		msgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		msgCreateInfo.pNext = VK_NULL_HANDLE;
		msgCreateInfo.flags = 0;
		msgCreateInfo.messageSeverity = (
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
		msgCreateInfo.messageType = (
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
		msgCreateInfo.pfnUserCallback = &DebugCallback;
		msgCreateInfo.pUserData = VK_NULL_HANDLE;

		return msgCreateInfo;
	}
#endif

	VkInstance VulkanContext::s_Instance = VK_NULL_HANDLE;

	VulkanContext::~VulkanContext()
	{
		YM_CORE_TRACE("Destroying vulkan context...")

		auto& device = VulkanDevice::Get().GetDevice();
		vkDeviceWaitIdle(device);

		YM_CORE_TRACE("Destroying vulkan swapchain framebuffer...")
		for (auto framebuffer : m_SCFramebuffers) {
			framebuffer.CleanUp();
		}

		VulkanSwapchain::Release();

		VulkanSurface::Release();

		YM_CORE_TRACE("Destroying vulkan sync objs...")
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(device, m_SignalSemaphores[i], VK_NULL_HANDLE);
			vkDestroySemaphore(device, m_WaitSemaphores[i], VK_NULL_HANDLE);
			vkDestroyFence(device, m_InFlightFences[i], VK_NULL_HANDLE);
		}

		m_CommandBuffers.Free();

		m_RenderPass.reset();
	
		VulkanDevice::Release();

	#ifdef YM_DEBUG
		YM_CORE_TRACE("Destroying vulkan debugger...")
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugUtilsMessengerEXT");
		YM_CORE_VERIFY(vkDestroyDebugUtilsMessenger, "Cannot find address of vkDestroyDebugUtilsMessengerEXT")
		vkDestroyDebugUtilsMessenger(s_Instance, m_Debugger, VK_NULL_HANDLE);
	#endif

		YM_CORE_TRACE("Destroying vulkan instance...")
		vkDestroyInstance(s_Instance, VK_NULL_HANDLE);
	}

	void VulkanContext::Init(void* p_Window)
	{
		YM_CORE_VERIFY(p_Window)
		m_Window = (GLFWwindow*)p_Window;

		YM_CORE_TRACE("Initializing vulkan context...")
		CreateInstance();

	#ifdef YM_DEBUG	
		{
			YM_CORE_TRACE("Creating vulkan debugger...")
			VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo = CreateDebugMessengerInfo();

			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
			vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkCreateDebugUtilsMessengerEXT");
			YM_CORE_VERIFY(vkCreateDebugUtilsMessenger, "Cannot find address of vkCreateDebugUtilsMessenger")

			auto res = vkCreateDebugUtilsMessenger(s_Instance, &msgCreateInfo, VK_NULL_HANDLE, &m_Debugger);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}
	#endif

		YM_CORE_TRACE("Creating vulkan surface...")
		VulkanSurface::Get().Init(p_Window);

		YM_CORE_TRACE("Creating vulkan logical device...")
		VulkanDevice::Get().Init();

		m_CommandBuffers.Init(MAX_FRAMES_IN_FLIGHT);

		YM_CORE_TRACE("Creating vulkan swapchain...")
		VulkanSwapchain::Get().Init(false /* Vsync */, m_Window, {800, 600});

		YM_CORE_TRACE("Creating vulkan renderpass...")
		m_RenderPass = CreateRef<VulkanRenderPass>();
		m_RenderPass->Init();

		for (const auto& imageView : VulkanSwapchain::Get().GetImageViews())
		{
			YM_CORE_TRACE("Creating vulkan swapchain framebuffer...")
			VulkanSCFramebuffer framebuffer;
			framebuffer.Init(m_RenderPass->Get(), imageView);
			m_SCFramebuffers.push_back(framebuffer);
		}

		CreateSyncObjs();
	}

	void VulkanContext::SwapBuffer()
	{	
		if (m_HasDrawCommands)
		{

			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			VkSemaphore waitSemaphores[] = { m_WaitSemaphores[m_CurrentFrame] };
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = waitSemaphores;

			VkSwapchainKHR swapChains[] = { VulkanSwapchain::Get().GetSwapChain() };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &m_CurrentImageIndex;
			presentInfo.pResults = nullptr; // Optional

			auto& presentQueue = VulkanDevice::Get().GetPresentQueue();
			auto res = vkQueuePresentKHR(presentQueue, &presentInfo);

			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_ViewportResized)
			{
				RecreateSwapchain();
			}
			else
			{
				YM_CORE_VERIFY(res == VK_SUCCESS)
			}
		}
		else if (m_ViewportResized)
		{
			RecreateSwapchain();
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		m_HasDrawCommands = false; // Reset the flag for the next frame
	}

	void VulkanContext::RecreateSwapchain()
	{
		auto& device = VulkanDevice::Get().GetDevice();

		VkExtent2D actualExtent;

		int width = 0, height = 0;
		if (m_ViewportResized)
		{
			YM_CORE_TRACE("Resized viewport...")

			glfwGetFramebufferSize(m_Window, &width, &height);
			while (width == 0 || height == 0) {
				glfwGetFramebufferSize(m_Window, &width, &height);
				glfwWaitEvents();
			}

			m_ViewportResized = false;
		}
		else
		{
			auto extent = VulkanSwapchain::Get().GetExtent2D();
			width = extent.width;
			height = extent.height;
		}

		actualExtent.width = (uint32_t)width;
		actualExtent.height = (uint32_t)height;

		vkDeviceWaitIdle(device);

		VulkanSwapchain::Get().Invalidade(actualExtent.width, actualExtent.height);
		for (auto i = 0; i < VulkanSwapchain::Get().GetImageViews().size(); i++)
		{
			m_SCFramebuffers[i].Invalidate(VulkanSwapchain::Get().GetImageViews()[i], actualExtent.width, actualExtent.height);
		}
	}

	void VulkanContext::WaitFence()
	{
		auto& device = VulkanDevice::Get().GetDevice();
		vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
	}

	void VulkanContext::ResetFence()
	{
		auto& device = VulkanDevice::Get().GetDevice();
		vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);
	}


	void VulkanContext::TransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout) const 
	{
		auto commandPool = VulkanDevice::Get().GetCommandPool();
		auto& device = VulkanDevice::Get().GetDevice();

		// Alocação do Command Buffer
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		// Início do Command Buffer
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		// Configuração da Barrier
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = p_OldLayout;
		barrier.newLayout = p_NewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = p_Image;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		if (p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || p_Format == VK_FORMAT_D24_UNORM_S8_UINT) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		// Determinação dos Estágios e Máscaras de Acesso
		if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (p_OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (p_OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (p_OldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else if (p_OldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && p_NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		// Pipeline Barrier
		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// Finalização do Command Buffer
		vkEndCommandBuffer(commandBuffer);

		// Submissão do Command Buffer
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		auto& graphicQueue = VulkanDevice::Get().GetGraphicQueue();

		vkQueueSubmit(graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicQueue);

		// Liberação do Command Buffer
		vkFreeCommandBuffers(device, VulkanDevice::Get().GetCommandPool(), 1, &commandBuffer);
	}

	void VulkanContext::BeginFrame()
	{
		auto& device = VulkanDevice::Get().GetDevice();
		vkWaitForFences(device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &m_InFlightFences[m_CurrentFrame]);

		// Semáforos
		VkSemaphore& signalSemaphore = m_SignalSemaphores[m_CurrentFrame];

		uint32_t imageIndex;
	
		if (auto res = vkAcquireNextImageKHR(device, VulkanSwapchain::Get().GetSwapChain(), UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &imageIndex);
			res == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchain();
			return;
		}
		else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			YM_CORE_VERIFY(false)
		}

		m_CurrentImageIndex = imageIndex;
	}

	void VulkanContext::CreateInstance()
	{
		YM_CORE_TRACE("Creating vulkan instance...")

		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = VK_NULL_HANDLE,
			.pApplicationName = "Sandbox",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "YUME engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_MAKE_VERSION(1, 283, 0)
		};

		std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation",
		};

		std::vector<const char*> requiredValidationLayers;

		YM_CORE_TRACE("Searching for required layers...")
		for (auto layer : validationLayers)
		{
			if (CheckLayerSupport(layer))
			{
				YM_CORE_INFO("Extension {} found!", layer)
				requiredValidationLayers.push_back(layer);
			}
			else
			{
				YM_CORE_ERROR("Required layer {} is missing!", layer)
			}
		}
		std::cout << "\n";

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> requiredExtensions;

		YM_CORE_TRACE("Searching for required extensions...")

		for (uint32_t i = 0; i < glfwExtensionCount; i++)
		{
			if (CheckExtensionSupport(glfwExtensions[i]))
			{
				YM_CORE_INFO("Extension {} found!", glfwExtensions[i])
				requiredExtensions.push_back(glfwExtensions[i]);
			}
			else
			{
				YM_CORE_ERROR("Required extension {} is missing!", glfwExtensions[i])
			}
		}

	#ifdef YM_DEBUG
		if (CheckExtensionSupport(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		{
			YM_CORE_INFO("Extension {} found!", VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		else
		{
			YM_CORE_ERROR("Required extension {} is missing!", VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
		}
	#endif


		VkInstanceCreateInfo instInfo = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.flags = 0,
			.pApplicationInfo = &appInfo,			
		#ifdef YM_DEBUG			
			.enabledLayerCount = (uint32_t)requiredValidationLayers.size(),
			.ppEnabledLayerNames = requiredValidationLayers.data(),
		#else
			.enabledLayerCount = 0,
		#endif
			.enabledExtensionCount = (uint32_t)requiredExtensions.size(),
			.ppEnabledExtensionNames = requiredExtensions.data(),
		};

	#ifdef YM_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT msgInfo = CreateDebugMessengerInfo();
		instInfo.pNext = &msgInfo;
	#endif

		auto res = vkCreateInstance(&instInfo, VK_NULL_HANDLE, &s_Instance);
		YM_CORE_VERIFY(res == VK_SUCCESS, "Failed to create instance!")
	}

	void VulkanContext::CreateSyncObjs()
	{
		auto& device = VulkanDevice::Get().GetDevice();

		m_SignalSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_WaitSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);


		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			auto res = vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_SignalSemaphores[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)
			res = vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_WaitSemaphores[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)

			res = vkCreateFence(device, &fenceInfo, VK_NULL_HANDLE, &m_InFlightFences[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)
		}
	}

	bool VulkanContext::CheckExtensionSupport(const char* p_Extension) const
	{
		uint32_t extensionCount = 0;
		auto res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		YM_CORE_ASSERT(res == VK_SUCCESS, "Check extension support has failed!")
		YM_CORE_ASSERT(extensionCount > 0)

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
		YM_CORE_ASSERT(res == VK_SUCCESS, "Check extension support has failed!")
		YM_CORE_ASSERT(!availableExtensions.empty())

		for (const auto& extension : availableExtensions)
		{
			if (strcmp(p_Extension, extension.extensionName) == 0)
				return true;
		}

		return false;
	}

	bool VulkanContext::CheckLayerSupport(const char* p_Layer) const
	{
		uint32_t layerCount = 0;
		auto res = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		YM_CORE_ASSERT(res == VK_SUCCESS, "Check layer support has failed!")
		YM_CORE_ASSERT(layerCount > 0)

		std::vector<VkLayerProperties> availableLayers(layerCount);
		res = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		YM_CORE_ASSERT(res == VK_SUCCESS, "Check layer support has failed!")
		YM_CORE_ASSERT(!availableLayers.empty())

		for (const auto& layer : availableLayers)
		{
			if (strcmp(p_Layer, layer.layerName) == 0)
				return true;
		}

		return false;
	}
}