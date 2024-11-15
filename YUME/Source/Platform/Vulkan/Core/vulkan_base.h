#pragma once
#include "YUME/Core/base.h"
#include <vulkan/vulkan.h>

inline PFN_vkCmdBeginDebugUtilsLabelEXT fpCmdBeginDebugUtilsLabelEXT;
inline PFN_vkCmdEndDebugUtilsLabelEXT fpCmdEndDebugUtilsLabelEXT;
inline PFN_vkSetDebugUtilsObjectNameEXT fpSetDebugUtilsObjectNameEXT;

#define USE_VMA_ALLOCATOR
//#define USE_SMALL_VMA_POOL
#ifdef USE_VMA_ALLOCATOR
	#define VMA_DEBUG
	#define VMA_DEBUG_LOG_ENABLE 0
	#define VMA_LEAK_LOG_ENABLE 1

	#if defined(VMA_DEBUG) && defined(YM_DEBUG)
		#define VMA_DEBUG_MARGIN 16
		#define VMA_DEBUG_DETECT_CORRUPTION 1

		#if VMA_DEBUG_LOG_ENABLE
			static char VMA_DEBUG_LOG_BUFFER[100];
			#define VMA_DEBUG_LOG_FORMAT(...)			    \
				sprintf(VMA_DEBUG_LOG_BUFFER, __VA_ARGS__); \
				YM_CORE_TRACE(VULKAN_PREFIX + std::string((const char*)VMA_DEBUG_LOG_BUFFER))
		#endif

		#if VMA_LEAK_LOG_ENABLE
			static char VMA_LEAK_LOG_BUFFER[100];
			#define VMA_LEAK_LOG_FORMAT(...)			    \
				sprintf(VMA_LEAK_LOG_BUFFER, __VA_ARGS__);  \
				YM_CORE_WARN(VULKAN_PREFIX + std::string((const char*)VMA_LEAK_LOG_BUFFER))
		#endif

	#endif

	#include <vk_mem_alloc.h>

	#ifdef USE_SMALL_VMA_POOL
		static const uint32_t SMALL_ALLOCATION_MAX_SIZE = 4096;
	#endif
#endif
