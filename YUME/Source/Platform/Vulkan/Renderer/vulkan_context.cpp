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
	#define YM_VK_DEBUG_LOG(Type)																	\
		YM_CORE_##Type(VULKAN_PREFIX "Debug Callback:")												\
		YM_CORE_##Type(VULKAN_PREFIX " Message: {}", p_CallbackData->pMessage)						\
		YM_CORE_##Type(VULKAN_PREFIX " Severity: {}", VKUtils::GetMessageSeverityStr(p_Severity))	\
		YM_CORE_##Type(VULKAN_PREFIX " Type: {}", VKUtils::GetMessageType(p_Type))					\
		YM_CORE_##Type(VULKAN_PREFIX " Objects: ")													\
																									\
		for (uint32_t i = 0; i < p_CallbackData->objectCount; i++) {								\
			YM_CORE_##Type("\t\t\t {}", p_CallbackData->pObjects[i].objectHandle)					\
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

	VulkanContext::~VulkanContext()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying context...")

		VKUtils::WaitIdle();

		VulkanSwapchain::Release();

		m_MainDeletionQueue.Flush();
	
		VulkanDevice::Release();

	#ifdef YM_DEBUG
		YM_CORE_TRACE(VULKAN_PREFIX "Destroying debugger...")
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkDestroyDebugUtilsMessengerEXT");
		YM_CORE_VERIFY(vkDestroyDebugUtilsMessenger, "Cannot find address of vkDestroyDebugUtilsMessengerEXT")
		vkDestroyDebugUtilsMessenger(s_Instance, m_Debugger, VK_NULL_HANDLE);
	#endif

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying instance...")
		vkDestroyInstance(s_Instance, VK_NULL_HANDLE);
	}

	void VulkanContext::Init(const char* p_Name, void* p_Window)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_ASSERT(p_Window)
		m_Window = (GLFWwindow*)p_Window;

		YM_CORE_TRACE(VULKAN_PREFIX "Initializing context...")
		CreateInstance(p_Name);

	#ifdef YM_DEBUG	
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Creating debugger...")
			VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo = CreateDebugMessengerInfo();

			PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
			vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(s_Instance, "vkCreateDebugUtilsMessengerEXT");
			YM_CORE_VERIFY(vkCreateDebugUtilsMessenger, "Cannot find address of vkCreateDebugUtilsMessenger")

			auto res = vkCreateDebugUtilsMessenger(s_Instance, &msgCreateInfo, VK_NULL_HANDLE, &m_Debugger);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}

		fpSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)(vkGetInstanceProcAddr(s_Instance, "vkSetDebugUtilsObjectNameEXT"));
		if (fpSetDebugUtilsObjectNameEXT == nullptr)
			fpSetDebugUtilsObjectNameEXT = [](VkDevice p_Device, const VkDebugUtilsObjectNameInfoEXT* p_NameInfo)
			{ return VK_SUCCESS; };

		fpCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)(vkGetInstanceProcAddr(s_Instance, "vkCmdBeginDebugUtilsLabelEXT"));
		if (fpCmdBeginDebugUtilsLabelEXT == nullptr)
			fpCmdBeginDebugUtilsLabelEXT = [](VkCommandBuffer p_CommandBuffer, const VkDebugUtilsLabelEXT* p_LabelInfo) {};

		fpCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)(vkGetInstanceProcAddr(s_Instance, "vkCmdEndDebugUtilsLabelEXT"));
		if (fpCmdEndDebugUtilsLabelEXT == nullptr)
			fpCmdEndDebugUtilsLabelEXT = [](VkCommandBuffer p_CommandBuffer) {};

	#else
		fpSetDebugUtilsObjectNameEXT = [](VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) { return VK_SUCCESS; };
		fpCmdBeginDebugUtilsLabelEXT = [](VkCommandBuffer p_CommandBuffer, const VkDebugUtilsLabelEXT* p_LabelInfo) {};
		fpCmdEndDebugUtilsLabelEXT = [](VkCommandBuffer p_CommandBuffer) {};
	#endif

		YM_CORE_TRACE(VULKAN_PREFIX "Creating logical device...")
		VulkanDevice::Get().Init();

		YM_CORE_TRACE(VULKAN_PREFIX "Creating swapchain...")
		VulkanSwapchain::Get().Init(false /* Vsync */, m_Window);

	#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)
		YM_CORE_TRACE(VULKAN_PREFIX "Initializing gpu optick...")

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
	}

	void VulkanContext::Begin()
	{
		YM_PROFILE_FUNCTION()
		VKUtils::WaitIdle();

		m_MainDeletionQueue.Flush();

		VulkanSwapchain::Get().Begin();

	}

	void VulkanContext::End()
	{
		YM_PROFILE_FUNCTION()

		VulkanSwapchain::Get().End();
	}

	void VulkanContext::SwapBuffer()
	{
		YM_PROFILE_FUNCTION()

		VulkanSwapchain::Get().Present();
	}

	void VulkanContext::OnResize(uint32_t p_Width, uint32_t p_Height)
	{
		YM_PROFILE_FUNCTION()
		if (p_Width == 0 || p_Height == 0)
			return;

		YM_CORE_INFO("VulkanContext::OnResize w: {}, h: {}", p_Width, p_Height)
		VulkanSwapchain::Get().OnResize(p_Width, p_Height);
	}

	CommandBuffer* VulkanContext::GetCurrentCommandBuffer()
	{
		return VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer.get();
	}


	void VulkanContext::CreateInstance(const char* p_Name)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Creating instance...")

		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = VK_NULL_HANDLE,
			.pApplicationName = p_Name,
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "YUME Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_MAKE_VERSION(1, 283, 0)
		};

		std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> requiredValidationLayers;

		YM_CORE_TRACE(VULKAN_PREFIX "Searching for required layers...")
		for (auto layer : validationLayers)
		{
			if (CheckLayerSupport(layer))
			{
				YM_CORE_INFO(VULKAN_PREFIX "Extension {} found!", layer)
				requiredValidationLayers.push_back(layer);
			}
			else
			{
				YM_CORE_ERROR(VULKAN_PREFIX "Required layer {} is missing!", layer)
			}
		}
		std::cout << "\n";

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> requiredExtensions;

		YM_CORE_TRACE(VULKAN_PREFIX "Searching for required extensions...")

		for (uint32_t i = 0; i < glfwExtensionCount; i++)
		{
			if (CheckExtensionSupport(glfwExtensions[i]))
			{
				YM_CORE_INFO(VULKAN_PREFIX "Extension {} found!", glfwExtensions[i])
				requiredExtensions.push_back(glfwExtensions[i]);
			}
			else
			{
				YM_CORE_ERROR(VULKAN_PREFIX "Required extension {} is missing!", glfwExtensions[i])
			}
		}

	#ifdef YM_DEBUG
		if (CheckExtensionSupport(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		{
			YM_CORE_INFO(VULKAN_PREFIX "Extension {} found!", VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		else
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Required extension {} is missing!", VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
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
		YM_CORE_VERIFY(res == VK_SUCCESS, VULKAN_PREFIX "Failed to create instance!")
	}

	bool VulkanContext::CheckExtensionSupport(const char* p_Extension) const
	{
		uint32_t extensionCount = 0;
		auto res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		YM_CORE_ASSERT(res == VK_SUCCESS, VULKAN_PREFIX "Check extension support has failed!")
		YM_CORE_ASSERT(extensionCount > 0)

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
		YM_CORE_ASSERT(res == VK_SUCCESS, VULKAN_PREFIX "Check extension support has failed!")
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
		YM_CORE_ASSERT(res == VK_SUCCESS, VULKAN_PREFIX "Check layer support has failed!")
		YM_CORE_ASSERT(layerCount > 0)

		std::vector<VkLayerProperties> availableLayers(layerCount);
		res = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		YM_CORE_ASSERT(res == VK_SUCCESS, VULKAN_PREFIX "Check layer support has failed!")
		YM_CORE_ASSERT(!availableLayers.empty())

		for (const auto& layer : availableLayers)
		{
			if (strcmp(p_Layer, layer.layerName) == 0)
				return true;
		}

		return false;
	}
}