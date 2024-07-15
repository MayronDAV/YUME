#pragma once

#include <memory>
#include <functional>



#ifdef YM_PLATFORM_WINDOWS
	#ifdef YM_BUILD_DLL
		#define YM_API __declspec(dllexport)
	#else
		#define YM_API __declspec(dllimport)
	#endif
#else
	#if YM_PLATFORM_LINUX
		#ifdef YM_BUILD_DLL
			#define YM_API __attribute__((visibility("default")))
		#else
			#define YM_API
		#endif
	#else
		#error Currently only support windows and linux!
	#endif
#endif

#define YM_EXPAND_MACRO(x) x
#define YM_STRINGIFY_MACRO(x) #x

#define YM_BIT(x) (1 << x)

#define YM_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#include "YUME/Core/assert.h"