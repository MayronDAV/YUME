#pragma once



#if defined(YM_PLATFORM_WINDOWS) && defined(YM_PROFILE)
	// Forward declaration to avoid including optick.h and vulkan.h in this header file   

	namespace Optick
	{
		struct VulkanFunctions;
	}

	struct VkDevice_T;
	struct VkPhysicalDevice_T;
	struct VkQueue_T;
	struct VkSwapchainKHR_T;

	using VkDevice = VkDevice_T*;
	using VkPhysicalDevice = VkPhysicalDevice_T*;
	using VkQueue = VkQueue_T*;
	using VkSwapchainKHR = VkSwapchainKHR_T*;

	namespace YUME
	{
		#define OPTICK_FRAME(NAME, ...)
		#define OPTICK_GPU_CONTEXT(BUFFER)
		#define OPTICK_TAG(NAME, ...)

		class YM_API Profile
		{
			public:
				template<typename... Args>
				static void Frame(const char* p_Name, Args&& ...p_Args)
				{
					OPTICK_FRAME(p_Name, std::forward<Args>(p_Args)...);
				}

				template <typename buffer>
				static void GPUContext(buffer p_CmdBuffer)
				{
					OPTICK_GPU_CONTEXT(p_CmdBuffer);
				}

				template<typename... Args>
				static void Tag(const char* p_Name, Args&& ...p_Args)
				{
					OPTICK_TAG(p_Name, std::forward<Args>(p_Args)...);
				}

				static void Function();
				static void Scope(const char* p_Name);
				static void Shutdown();
				static void GPUInitVulkan(VkDevice* p_Devices, VkPhysicalDevice* p_PhysicalDevices, VkQueue* p_CmdQueues, uint32_t* p_CmdQueuesFamily, uint32_t p_NumCmdQueues, const Optick::VulkanFunctions* p_Functions);

				static void GPUFlip(VkSwapchainKHR p_Swapchain);
		};
	}


	#define YM_PROFILE_FRAME(NAME, ...) ::YUME::Profile::Frame(NAME, ##__VA_ARGS__);
	#define YM_PROFILE_FUNCTION() ::YUME::Profile::Function();
	#define YM_PROFILE_SCOPE(NAME) ::YUME::Profile::Scope(NAME);
	#define YM_PROFILE_SHUTDOWN() ::YUME::Profile::Shutdown();
	#define YM_PROFILE_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS)  \
			 ::YUME::Profile::GPUInitVulkan(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS);
	#define YM_PROFILE_GPU_CONTEXT(cmdBuffer) YUME::Profile::GPUContext(cmdBuffer);
	#define YM_PROFILE_GPU_FLIP(swapchain) YUME::Profile::GPUFlip(swapchain);
	#define YM_PROFILE_TAG(NAME, ...) YUME::Profile::Tag(NAME, ##__VA_ARGS__);


	#undef OPTICK_FRAME
	#undef OPTICK_GPU_CONTEXT
	#undef OPTICK_TAG
#else
	#ifdef USE_OPTICK
	#undef USE_OPTICK
	#endif
	#define USE_OPTICK 0

	#define YM_PROFILE_FRAME(NAME, ...)
	#define YM_PROFILE_FUNCTION()
	#define YM_PROFILE_SCOPE(NAME)
	#define YM_PROFILE_SHUTDOWN()
	#define YM_PROFILE_GPU_INIT_VULKAN(DEVICES, PHYSICAL_DEVICES, CMD_QUEUES, CMD_QUEUES_FAMILY, NUM_CMD_QUEUS, FUNCTIONS)
	#define YM_PROFILE_GPU_CONTEXT(cmdBuffer)
	#define YM_PROFILE_GPU_FLIP(swapchain)
	#define YM_PROFILE_TAG(NAME, ...)

#endif
