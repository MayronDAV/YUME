#include "YUME/yumepch.h"
#include "vulkan_context.h"

#include "Platforms/Vulkan/Utils/vulkan_utils.h"

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


	VulkanContext::~VulkanContext()
	{
		vkDeviceWaitIdle(m_Device);

		DestroySwapchain();

		YM_CORE_TRACE("Destroying vulkan renderpass...")
		vkDestroyRenderPass(m_Device, m_RenderPass, VK_NULL_HANDLE);

		YM_CORE_TRACE("Destroying vulkan sync objs...")
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_Device, m_SignalSemaphores[i], VK_NULL_HANDLE);
			vkDestroySemaphore(m_Device, m_WaitSemaphores[i], VK_NULL_HANDLE);
			vkDestroyFence(m_Device, m_InFlightFences[i], VK_NULL_HANDLE);
		}

		YM_CORE_TRACE("Destroying vulkan command pool...")
		vkDestroyCommandPool(m_Device, m_CommandPool, VK_NULL_HANDLE);

		YM_CORE_TRACE("Destroying vulkan logical device...")
		vkDestroyDevice(m_Device, VK_NULL_HANDLE);

		YM_CORE_TRACE("Destroying vulkan surface...")
		vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);

	#ifdef YM_DEBUG
		YM_CORE_TRACE("Destroying vulkan debugger...")
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
		YM_CORE_VERIFY(vkDestroyDebugUtilsMessenger, "Cannot find address of vkDestroyDebugUtilsMessengerEXT")
		vkDestroyDebugUtilsMessenger(m_Instance, m_Debugger, VK_NULL_HANDLE);
	#endif

		YM_CORE_TRACE("Destroying vulkan instance...")
		vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
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
			vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
			YM_CORE_VERIFY(vkCreateDebugUtilsMessenger, "Cannot find address of vkCreateDebugUtilsMessenger")

			auto res = vkCreateDebugUtilsMessenger(m_Instance, &msgCreateInfo, VK_NULL_HANDLE, &m_Debugger);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}
	#endif

		YM_CORE_TRACE("Creating vulkan surface...")
		auto res = glfwCreateWindowSurface(m_Instance, m_Window, VK_NULL_HANDLE, &m_Surface);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		GetPhysicalDevices();
		SelectPhysicalDevice();

		YM_CORE_TRACE("Creating vulkan logical device...")
		CreateLogicalDevice();

		YM_CORE_TRACE("Creating vulkan swapchain...")
		CreateSwapchain();
		CreateImagesView();

		YM_CORE_TRACE("Creating vulkan renderpass...")
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_SwapchainOptions.Format.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		CreateRenderPass(&renderPassInfo);

		CreateSwapchainFramebuffer();

		CreateSyncObjs();

		YM_CORE_TRACE("Creating vulkan command pool...")
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_QueueFamily;

		CreateCommandPool(&poolInfo);
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

			VkSwapchainKHR swapChains[] = { m_SwapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &m_CurrentImageIndex;
			presentInfo.pResults = nullptr; // Optional

			auto res = vkQueuePresentKHR(m_Queue, &presentInfo);

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

	VkSwapchainKHR& VulkanContext::CreateSwapchain(const SwapchainOptions& p_Options)
	{
		m_SwapchainOptions = p_Options;

		const auto& physDevice = m_PhysicalDevices[m_SelectedDeviceIndex];
		uint32_t requestedNumImages = physDevice.SurfaceCapabilities.minImageCount + 1;

		uint32_t minNumImages = 0;

		if ((physDevice.SurfaceCapabilities.maxImageCount > 0) &&
			(requestedNumImages > physDevice.SurfaceCapabilities.maxImageCount))
		{
			minNumImages = physDevice.SurfaceCapabilities.maxImageCount;
		}
		else
		{
			minNumImages = requestedNumImages;
		}

		VkSwapchainCreateInfoKHR scInfo{};
		scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		scInfo.surface = m_Surface;
		scInfo.minImageCount = minNumImages;
		scInfo.imageFormat = p_Options.Format.format;
		scInfo.imageColorSpace = p_Options.Format.colorSpace;
		scInfo.imageExtent = p_Options.Extent2D;
		scInfo.imageArrayLayers = 1;
		scInfo.imageUsage = p_Options.Usage;
		scInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		scInfo.queueFamilyIndexCount = 1;
		scInfo.pQueueFamilyIndices = &m_QueueFamily;
		scInfo.preTransform = physDevice.SurfaceCapabilities.currentTransform;
		scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // TODO: maybe make this configurable?
		scInfo.presentMode = p_Options.PresentMode;
		scInfo.clipped = VK_TRUE;

		auto res = vkCreateSwapchainKHR(m_Device, &scInfo, VK_NULL_HANDLE, &m_SwapChain);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		res = vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_ImagesCount, nullptr);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(m_ImagesCount > 0)
		
		m_Images.resize(m_ImagesCount);
		res = vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_ImagesCount, m_Images.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(!m_Images.empty())

		return m_SwapChain;
	}

	std::vector<VkImageView>& VulkanContext::CreateImagesView(const ImagesViewOptions& p_Options)
	{
		m_ImageViewsOptions = p_Options;

		uint32_t swapchainImagesCount = 0;
		auto res = vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &swapchainImagesCount, VK_NULL_HANDLE);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(swapchainImagesCount > 0)

		m_ImagesView.resize(swapchainImagesCount);

		for (uint32_t i = 0; i < swapchainImagesCount; i++)
		{
			VkImageViewCreateInfo imgViewInfo{};
			imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgViewInfo.pNext = VK_NULL_HANDLE;
			imgViewInfo.flags = 0;
			imgViewInfo.image = m_Images[i];
			imgViewInfo.viewType = p_Options.ViewType;
			imgViewInfo.format = p_Options.Format.format;
			imgViewInfo.components = p_Options.Component;
			imgViewInfo.subresourceRange = p_Options.SubresourceRange;

			res = vkCreateImageView(m_Device, &imgViewInfo, VK_NULL_HANDLE, &m_ImagesView[i]);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}

		return m_ImagesView;
	}

	VkRenderPass& VulkanContext::CreateRenderPass(const VkRenderPassCreateInfo* p_Info)
	{
		m_RenderPassInitialImageLayout = p_Info->pAttachments->initialLayout;
		m_RenderPassFinalImageLayout = p_Info->pAttachments->finalLayout;

		auto res = vkCreateRenderPass(m_Device, p_Info, VK_NULL_HANDLE, &m_RenderPass);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		return m_RenderPass;
	}

	std::vector<VkFramebuffer>& VulkanContext::CreateSwapchainFramebuffer()
	{
		m_SwapchainFramebuffers.resize(m_ImagesView.size());

		for (size_t i = 0; i < m_ImagesView.size(); i++) {
			VkImageView attachments[] = {
				m_ImagesView[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_RenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_SwapchainOptions.Extent2D.width;
			framebufferInfo.height = m_SwapchainOptions.Extent2D.height;
			framebufferInfo.layers = 1;

			auto res = vkCreateFramebuffer(m_Device, &framebufferInfo, VK_NULL_HANDLE, &m_SwapchainFramebuffers[i]);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}

		return m_SwapchainFramebuffers;
	}

	void VulkanContext::RecreateSwapchain()
	{
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
			width = m_SwapchainOptions.Extent2D.width;
			height = m_SwapchainOptions.Extent2D.height;
		}

		m_SwapchainOptions.Extent2D.width = (uint32_t)width;
		m_SwapchainOptions.Extent2D.height = (uint32_t)height;

		vkDeviceWaitIdle(m_Device);

		DestroySwapchain();

		YM_CORE_TRACE("Creating vulkan swapchain...")
		CreateSwapchain(m_SwapchainOptions);
		YM_CORE_TRACE("Creating vulkan swapchain images view...")
		CreateImagesView(m_ImageViewsOptions);
		YM_CORE_TRACE("Creating vulkan swapchain framebuffer...")
		CreateSwapchainFramebuffer();
	}

	void VulkanContext::WaitFence()
	{
		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
	}

	void VulkanContext::ResetFence()
	{
		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);
	}

	VkCommandPool& VulkanContext::CreateCommandPool(const VkCommandPoolCreateInfo* p_Info)
	{
		auto res = vkCreateCommandPool(m_Device, p_Info, VK_NULL_HANDLE, &m_CommandPool);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		ResetCommandBuffers();

		CreateCommandBuffers();

		return m_CommandPool;
	}

	void VulkanContext::TransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout)
	{
		// Alocação do Command Buffer
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

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

		vkQueueSubmit(m_Queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_Queue);

		// Liberação do Command Buffer
		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
	}


	void VulkanContext::BeginFrame()
	{
		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

		// Semáforos
		VkSemaphore& signalSemaphore = m_SignalSemaphores[m_CurrentFrame];

		uint32_t imageIndex;
		auto res = vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchain();
			return;
		}
		else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
			YM_CORE_VERIFY(false)
		}

		m_CurrentImageIndex = imageIndex;

		vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
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

		auto res = vkCreateInstance(&instInfo, VK_NULL_HANDLE, &m_Instance);
		YM_CORE_VERIFY(res == VK_SUCCESS, "Failed to create instance!")
	}

	void VulkanContext::GetPhysicalDevices()
	{
		uint32_t deviceCount = 0;
		auto res = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(deviceCount > 0)

		std::vector<VkPhysicalDevice> devices(deviceCount);
		res = vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(!devices.empty())

		m_PhysicalDevices.resize(deviceCount);

		for (uint32_t i = 0; i < deviceCount; i++)
		{
			VkPhysicalDevice device = devices[i];
			m_PhysicalDevices[i].Device = device;

			vkGetPhysicalDeviceProperties(device, &m_PhysicalDevices[i].Properties);

			vkGetPhysicalDeviceFeatures(device, &m_PhysicalDevices[i].Features);

			uint32_t familyPropsCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyPropsCount, nullptr);

			m_PhysicalDevices[i].FamilyProperties.resize(familyPropsCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &familyPropsCount, m_PhysicalDevices[i].FamilyProperties.data());

			m_PhysicalDevices[i].QueueSupportsPresent.resize(familyPropsCount);
			for (uint32_t q = 0; q < familyPropsCount; q++)
			{
				res = vkGetPhysicalDeviceSurfaceSupportKHR(device, q, m_Surface, m_PhysicalDevices[i].QueueSupportsPresent.data());
				YM_CORE_VERIFY(res == VK_SUCCESS)
			}

			uint32_t surfaceFormatsCount = 0;
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &surfaceFormatsCount, nullptr);
			YM_CORE_VERIFY(res == VK_SUCCESS)
			YM_CORE_VERIFY(surfaceFormatsCount > 0)

			m_PhysicalDevices[i].SurfaceFormats.resize(surfaceFormatsCount);
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &surfaceFormatsCount, m_PhysicalDevices[i].SurfaceFormats.data());
			YM_CORE_VERIFY(res == VK_SUCCESS)

			res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &m_PhysicalDevices[i].SurfaceCapabilities);
			YM_CORE_VERIFY(res == VK_SUCCESS)

			vkGetPhysicalDeviceMemoryProperties(device, &m_PhysicalDevices[i].MemoryProperties);

			uint32_t presentModesCount = 0;
			res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModesCount, nullptr);
			YM_CORE_VERIFY(res == VK_SUCCESS)
			YM_CORE_VERIFY(presentModesCount > 0)

			m_PhysicalDevices[i].PresentModes.resize(presentModesCount);
			res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModesCount, m_PhysicalDevices[i].PresentModes.data());
			YM_CORE_VERIFY(res == VK_SUCCESS)

			YM_CORE_TRACE("Physical devices finded:")
			YM_CORE_TRACE("\t ID: {}", m_PhysicalDevices[i].Properties.deviceID)
			YM_CORE_TRACE("\t Name: {}", m_PhysicalDevices[i].Properties.deviceName)
			YM_CORE_TRACE("\t Driver Version: {}", m_PhysicalDevices[i].Properties.driverVersion)
		}

		YM_CORE_VERIFY(!m_PhysicalDevices.empty())
	}

	void VulkanContext::SelectPhysicalDevice()
	{
		// TODO: Maybe revisit this
		YM_CORE_TRACE("Selecting vulkan physical device...")

		for (uint32_t i = 0; i < m_PhysicalDevices.size(); i++)
		{
			for (uint32_t j = 0; j < m_PhysicalDevices[i].FamilyProperties.size(); j++)
			{
				auto familyProps = m_PhysicalDevices[i].FamilyProperties[j];
				bool supportsPresent = m_PhysicalDevices[i].QueueSupportsPresent[j];

				if ((familyProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent)
				{
					YM_CORE_INFO(" Physical device founded: ")
					m_SelectedDeviceIndex = i;
					m_QueueFamily = j;
					YM_CORE_INFO("\t Name: {}", m_PhysicalDevices[i].Properties.deviceName)
					YM_CORE_INFO("\t Vendor ID: {}", m_PhysicalDevices[i].Properties.vendorID)
					YM_CORE_INFO("\t ID: {}", m_SelectedDeviceIndex)
					YM_CORE_INFO("\t Queue Family: {}\n", m_QueueFamily)
				}
			}
		}

		YM_CORE_VERIFY(true, "Not find any device with this props!")
	}

	void VulkanContext::CreateLogicalDevice()
	{
		float qPriorities[] = { 1.0f };
		const auto& physDevice = m_PhysicalDevices[m_SelectedDeviceIndex];

		VkDeviceQueueCreateInfo qInfo{};
		qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qInfo.queueCount = 1;
		qInfo.queueFamilyIndex = m_QueueFamily;
		qInfo.pQueuePriorities = &qPriorities[0];

		std::vector<const char*> devExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
		};
		std::vector<const char*> requiredExts;

		YM_CORE_TRACE("Checking device extensions...")
		for (auto ext : devExts)
		{
			if (CheckDeviceExtensionSupport(ext)) {
				YM_CORE_INFO("Device extension {} founded!", ext)
				requiredExts.push_back(ext);
			}
			else {
				YM_CORE_ERROR("Device extension {} not founded!", ext)
			}
		}
		std::cout << "\n";

		YM_CORE_ASSERT(physDevice.Features.geometryShader == VK_TRUE, "Gemotry shader isn't supported!")
		YM_CORE_ASSERT(physDevice.Features.tessellationShader == VK_TRUE, "Tesselation shader isn't supported!")

		VkPhysicalDeviceFeatures physFeatures{ 0 };
		physFeatures.geometryShader = VK_TRUE;
		physFeatures.tessellationShader = VK_TRUE;

		VkDeviceCreateInfo devInfo{};
		devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		devInfo.queueCreateInfoCount = 1;
		devInfo.pQueueCreateInfos = &qInfo;
		devInfo.enabledExtensionCount = (uint32_t)requiredExts.size();
		devInfo.ppEnabledExtensionNames = requiredExts.data();
		devInfo.pEnabledFeatures = &physFeatures;

		auto res = vkCreateDevice(physDevice.Device, &devInfo, VK_NULL_HANDLE, &m_Device);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		vkGetDeviceQueue(m_Device, m_QueueFamily, 0, &m_Queue);
	}

	void VulkanContext::CreateSyncObjs()
	{
		m_SignalSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_WaitSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);


		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCreateInfo.flags = VK_SEMAPHORE_WAIT_ANY_BIT;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			auto res = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_SignalSemaphores[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)
			res = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_WaitSemaphores[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)

			res = vkCreateFence(m_Device, &fenceInfo, VK_NULL_HANDLE, &m_InFlightFences[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)
		}
	}

	void VulkanContext::ResetCommandBuffers()
	{
		for (auto& buffer : m_CommandBuffers)
		{
			vkResetCommandBuffer(buffer, 0);
		}
	}

	void VulkanContext::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		auto res = vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data());
		YM_CORE_VERIFY(res == VK_SUCCESS);
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

	bool VulkanContext::CheckDeviceExtensionSupport(const char* p_Extension) const
	{
		uint32_t extensionCount = 0;
		const auto& physDevice = m_PhysicalDevices[m_SelectedDeviceIndex];
		auto res = vkEnumerateDeviceExtensionProperties(physDevice.Device, nullptr, &extensionCount, nullptr);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(extensionCount > 0)

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		res = vkEnumerateDeviceExtensionProperties(physDevice.Device, nullptr, &extensionCount, availableExtensions.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_VERIFY(!availableExtensions.empty())

		for (const auto& extension : availableExtensions)
		{
			if (strcmp(p_Extension, extension.extensionName) == 0)
				return true;
		}

		return false;
	}

	void VulkanContext::DestroySwapchain()
	{
		YM_CORE_TRACE("Destroying vulkan swapchain framebuffer...")
		for (auto framebuffer : m_SwapchainFramebuffers) {
			vkDestroyFramebuffer(m_Device, framebuffer, VK_NULL_HANDLE);
		}

		YM_CORE_TRACE("Destroying vulkan swapchain...")
		for (auto& imgView : m_ImagesView)
		{
			vkDestroyImageView(m_Device, imgView, VK_NULL_HANDLE);
		}

		vkDestroySwapchainKHR(m_Device, m_SwapChain, VK_NULL_HANDLE);
	}

}