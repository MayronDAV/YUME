#include "YUME/yumepch.h"
#include "vulkan_context.h"

#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "Platform/Vulkan/ImGui/vulkan_imgui_layer.h"
#include "YUME/Core/application.h"
#include "YUME/Renderer/texture.h"
#include "vulkan_texture.h"

// Lib
#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)
	#include <optick/optick.h>
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace YUME
{

#ifdef YM_DEBUG
	#define YM_VK_DEBUG_LOG(Type)															 \
		YM_CORE_##Type("Debug Callback:")													 \
		YM_CORE_##Type("{}", p_CallbackData->pMessage)										 \
		YM_CORE_##Type("\t Severity: {}", Utils::GetMessageSeverityStr(p_Severity))			 \
		YM_CORE_##Type("\t Type: {}", Utils::GetMessageType(p_Type))						 \
		YM_CORE_##Type("\t Objects: ")														 \
																							 \
		for (uint32_t i = 0; i < p_CallbackData->objectCount; i++) {						 \
			YM_CORE_##Type("\t\t {}", p_CallbackData->pObjects[i].objectHandle)				 \
		}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity,
		VkDebugUtilsMessageTypeFlagsEXT p_Type,
		const VkDebugUtilsMessengerCallbackDataEXT* p_CallbackData,
		void* p_UserData)
	{
		switch (p_Severity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: YM_VK_DEBUG_LOG(TRACE) break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:	  YM_VK_DEBUG_LOG(INFO)  break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: YM_VK_DEBUG_LOG(WARN)  break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   YM_VK_DEBUG_LOG(ERROR) break;
			default:
				YM_VK_DEBUG_LOG(TRACE)
				break;	
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
	DeletionQueue VulkanContext::m_MainDeletionQueue = DeletionQueue();
	DeletionQueue VulkanContext::m_FrameEndDeletionQueue = DeletionQueue();

	static std::vector<std::function<void(int, int)>> s_SwapchainOnResizeQueue;

	VulkanContext::~VulkanContext()
	{
		YM_PROFILE_FUNCTION()

		s_SwapchainOnResizeQueue.clear();

		YM_CORE_TRACE("Destroying vulkan context...")

		auto& device = VulkanDevice::Get().GetDevice();
		vkDeviceWaitIdle(device);

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

		m_MainDeletionQueue.Flush();
	
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
		YM_PROFILE_FUNCTION()

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

	#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)
		auto device = VulkanDevice::Get().GetDevice();
		auto physicalDevice = VulkanDevice::Get().GetPhysicalDevice();
		auto queue = VulkanDevice::Get().GetGraphicQueue();
		auto graphicIndex = (uint32_t)VulkanDevice::Get().GetPhysicalDeviceStruct().Indices.Graphics;

		Optick::VulkanFunctions vulkanFunctions{};
		vulkanFunctions.vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties_)vkGetPhysicalDeviceProperties;
		vulkanFunctions.vkCreateQueryPool = (PFN_vkCreateQueryPool_)vkCreateQueryPool;
		vulkanFunctions.vkCreateCommandPool = (PFN_vkCreateCommandPool_)vkCreateCommandPool;
		vulkanFunctions.vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers_)vkAllocateCommandBuffers;
		vulkanFunctions.vkCreateFence = (PFN_vkCreateFence_)vkCreateFence;
		vulkanFunctions.vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool_)vkCmdResetQueryPool;
		vulkanFunctions.vkQueueSubmit = (PFN_vkQueueSubmit_)vkQueueSubmit;
		vulkanFunctions.vkWaitForFences = (PFN_vkWaitForFences_)vkWaitForFences;
		vulkanFunctions.vkResetCommandBuffer = (PFN_vkResetCommandBuffer_)vkResetCommandBuffer;
		vulkanFunctions.vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp_)vkCmdWriteTimestamp;
		vulkanFunctions.vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults_)vkGetQueryPoolResults;
		vulkanFunctions.vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer_)vkBeginCommandBuffer;
		vulkanFunctions.vkEndCommandBuffer = (PFN_vkEndCommandBuffer_)vkEndCommandBuffer;
		vulkanFunctions.vkResetFences = (PFN_vkResetFences_)vkResetFences;
		vulkanFunctions.vkDestroyCommandPool = (PFN_vkDestroyCommandPool_)vkDestroyCommandPool;
		vulkanFunctions.vkDestroyQueryPool = (PFN_vkDestroyQueryPool_)vkDestroyQueryPool;
		vulkanFunctions.vkDestroyFence = (PFN_vkDestroyFence_)vkDestroyFence;
		vulkanFunctions.vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers_)vkFreeCommandBuffers;

		YM_PROFILE_GPU_INIT_VULKAN(&device, &physicalDevice, &queue, &graphicIndex, 1, &vulkanFunctions)
	#endif

		m_CommandBuffers.Init(MAX_FRAMES_IN_FLIGHT);

		YM_CORE_TRACE("Creating vulkan swapchain...")
		VulkanSwapchain::Get().Init(false /* Vsync */, m_Window);

		CreateSyncObjs();
	}

	void VulkanContext::SwapBuffer()
	{	
		YM_PROFILE_FUNCTION()

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

			YM_PROFILE_GPU_FLIP(VulkanSwapchain::Get().GetSwapChain())
#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)
			OPTICK_CATEGORY("Present", Optick::Category::Wait)
#endif
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

#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)
		OPTICK_GPU_CONTEXT(m_CommandBuffers.Get(m_CurrentFrame))
#endif
	}

	void VulkanContext::RecreateSwapchain()
	{
		YM_PROFILE_FUNCTION()

		// TODO: Make this class not block the main loop if the width or height is 0
		// instead, just don't recreate and don't call other things like vkAcquireNextImageKHR.

		auto& device = VulkanDevice::Get().GetDevice();
		auto& physdevice = VulkanDevice::Get().GetPhysicalDevice();

		VkExtent2D actualExtent;
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physdevice, VulkanSurface::Get().GetSurface(), &capabilities);

		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			actualExtent = capabilities.currentExtent;
		}

		if (capabilities.currentExtent.width == UINT32_MAX || (actualExtent.width == 0 || actualExtent.height == 0))
		{
			int width, height;
			glfwGetFramebufferSize(m_Window, &width, &height);
			while (width == 0 || height == 0) {
				glfwGetFramebufferSize(m_Window, &width, &height);
				glfwWaitEvents();
			}

			actualExtent = {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height)) 
			};
		}

		vkDeviceWaitIdle(device);

		VulkanSwapchain::Get().Invalidade(actualExtent.width, actualExtent.height);

		Application::Get().GetImGuiLayer()->OnResize(actualExtent.width, actualExtent.height);

		const auto& images = VulkanSwapchain::Get().GetImages();
		const auto& imageViews = VulkanSwapchain::Get().GetImageViews();

		for (auto it = s_SwapchainOnResizeQueue.rbegin(); it != s_SwapchainOnResizeQueue.rend(); it++) {
			(*it)(actualExtent.width, actualExtent.height); //call functors
		}
		//s_SwapchainOnResizeQueue.clear();

		m_ViewportResized = false;
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

	void VulkanContext::BeginFrame()
	{
		YM_PROFILE_FUNCTION()

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

		auto images = VulkanSwapchain::Get().GetImages();
		Utils::TransitionImageLayout(images[m_CurrentFrame], VulkanSwapchain::Get().GetFormat().format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_CommandBuffers.Reset(m_CurrentFrame);

		m_CommandBuffers.Begin(m_CurrentFrame);
	}

	void VulkanContext::EndFrame()
	{
		YM_PROFILE_FUNCTION()

		auto images = VulkanSwapchain::Get().GetImages();
		Utils::TransitionImageLayout(images[m_CurrentFrame], VulkanSwapchain::Get().GetFormat().format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		m_CommandBuffers.End(m_CurrentFrame);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		VkSemaphore waitSemaphores[] = { m_SignalSemaphores[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffers.Get(m_CurrentFrame);

		submitInfo.signalSemaphoreCount = 1;
		VkSemaphore signalSemaphores[] = { m_WaitSemaphores[m_CurrentFrame] };
		submitInfo.pSignalSemaphores = signalSemaphores;

		auto res = vkQueueSubmit(VulkanDevice::Get().GetGraphicQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		m_FrameEndDeletionQueue.Flush();
	}

	void VulkanContext::PushFunctionToSwapchainOnResizeQueue(const std::function<void(int, int)>& p_Function)
	{
		s_SwapchainOnResizeQueue.push_back(p_Function);
	}

	void VulkanContext::CreateInstance()
	{
		YM_PROFILE_FUNCTION()

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
			"VK_LAYER_KHRONOS_validation"
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
		YM_PROFILE_FUNCTION()

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