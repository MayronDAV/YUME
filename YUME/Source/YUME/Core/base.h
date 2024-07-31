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

#define YM_NO_DLLINTERFACE_WARN 1

#define YM_EXPAND_MACRO(x) x
#define YM_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define YM_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)


#ifndef YM_DEBUG
	#ifdef YM_PLATFORM_LINUX
		#define YM_ALWAYS_INLINE __attribute__((always_inline)) inline
	#elif defined(YM_PLATFORM_WINDOWS)
		#define YM_ALWAYS_INLINE __forceinline
	#endif
#else
	#define YM_ALWAYS_INLINE inline
#endif

#define YM_FORCE_INLINE YM_ALWAYS_INLINE

#define YM_NONCOPYABLE(class_name)                     \
    class_name(const class_name&)            = delete; \
    class_name& operator=(const class_name&) = delete;

#define YM_NONCOPYABLEANDMOVE(class_name)              \
    class_name(const class_name&)            = delete; \
    class_name& operator=(const class_name&) = delete; \
    class_name(class_name&&)                 = delete; \
    class_name& operator=(class_name&&)      = delete;

#include "YUME/Core/assert.h"

namespace YUME
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... p_Args)
	{
		return std::make_unique<T>(std::forward<Args>(p_Args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... p_Args)
	{
		return std::make_shared<T>(std::forward<Args>(p_Args)...);
	}

	template<typename T>
	using WeakRef = std::weak_ptr<T>;
}
