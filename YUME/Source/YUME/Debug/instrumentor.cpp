#include "YUME/yumepch.h"




#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)

// Lib
#include <vulkan/vulkan.h>
#include <optick/optick.h>

namespace YUME
{

	void Profile::Function()
	{
		OPTICK_EVENT();
	}

	void Profile::Scope(const char* p_Name)
	{
		OPTICK_CATEGORY(p_Name, Optick::Category::Debug);
	}

	void Profile::Shutdown()
	{
		OPTICK_SHUTDOWN();
	}

	void Profile::GPUInitVulkan(VkDevice* p_Devices, VkPhysicalDevice* p_PhysicalDevices, VkQueue* p_CmdQueues,
		uint32_t* p_CmdQueuesFamily, uint32_t p_NumCmdQueues, const Optick::VulkanFunctions* p_Functions)
	{
		OPTICK_GPU_INIT_VULKAN(p_Devices, p_PhysicalDevices, p_CmdQueues, p_CmdQueuesFamily, p_NumCmdQueues, p_Functions);
	}

	void Profile::GPUFlip(VkSwapchainKHR p_Swapchain)
	{
		OPTICK_GPU_FLIP(p_Swapchain);
	}
}

#endif
