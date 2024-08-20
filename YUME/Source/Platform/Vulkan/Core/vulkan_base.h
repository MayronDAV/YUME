#pragma once
#include "YUME/Core/base.h"
#include <vulkan/vulkan.h>

#define USE_VMA_ALLOCATOR
//#define USE_SMALL_VMA_POOL
#ifdef USE_VMA_ALLOCATOR
	#ifdef YM_DEBUG
		#define VMA_DEBUG_MARGIN 16
		#define VMA_DEBUG_DETECT_CORRUPTION 1

		static char VMA_DEBUG_LOG_BUFFER[100];
		#define VMA_DEBUG_LOG_FORMAT(...)			    \
			sprintf(VMA_DEBUG_LOG_BUFFER, __VA_ARGS__); \
			YM_CORE_TRACE((const char*)VMA_DEBUG_LOG_BUFFER)

		static char VMA_LEAK_LOG_BUFFER[100];
		#define VMA_LEAK_LOG_FORMAT(...)			    \
			sprintf(VMA_LEAK_LOG_BUFFER, __VA_ARGS__); \
			YM_CORE_WARN((const char*)VMA_LEAK_LOG_BUFFER)
	#endif

	#include <vk_mem_alloc.h>

	#ifdef USE_SMALL_VMA_POOL
		static const uint32_t SMALL_ALLOCATION_MAX_SIZE = 4096;
	#endif
#endif
