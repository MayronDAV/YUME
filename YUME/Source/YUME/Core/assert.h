#pragma once

#include "YUME/Core/log.h"
#include <filesystem>



#if defined(YM_PLATFORM_WINDOWS) && defined(_MSC_VER)
	#define YM_DEBUGBREAK() __debugbreak()
#else
	#if defined(YM_PLATFORM_LINUX) && (defined(__GNUC__) || defined(__clang__))
		#include <signal.h>
		#if defined(__i386__) || defined(__x86_64__)
			#define YM_DEBUGBREAK() __asm__ volatile("int3")
		#elif defined(__aarch64__) || defined(__arm__)
			#define YM_DEBUGBREAK() __builtin_trap()
		#else
			#include <csignal>
		#endif

		#if defined(SIGTRAP)
			#define YM_DEBUGBREAK() raise(SIGTRAP)
		#else
			#define YM_DEBUGBREAK() raise(SIGABRT)
		#endif
	#else
		#error YUME currently only support windows (msvc) and linux (gcc/clang)!
	#endif
#endif

#ifdef YM_DEBUG
	#define YM_ENABLE_ASSERTS
#endif

#ifndef YM_DIST
	#define YM_ENABLE_VERIFY
#endif


#ifdef YM_ENABLE_ASSERTS
	#define YM_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { YM##type##ERROR(msg, __VA_ARGS__); YM_DEBUGBREAK(); } }
	#define YM_INTERNAL_ASSERT_WITH_MSG(type, check, ...) YM_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define YM_INTERNAL_ASSERT_NO_MSG(type, check) YM_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", YM_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define YM_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define YM_INTERNAL_ASSERT_GET_MACRO(...) YM_EXPAND_MACRO( YM_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, YM_INTERNAL_ASSERT_WITH_MSG, YM_INTERNAL_ASSERT_NO_MSG) )

	#define YM_ASSERT(...) YM_EXPAND_MACRO( YM_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define YM_CORE_ASSERT(...) YM_EXPAND_MACRO( YM_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define YM_ASSERT(...)
	#define YM_CORE_ASSERT(...)
#endif

#ifdef YM_ENABLE_VERIFY
	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define YM_INTERNAL_VERIFY_IMPL(type, check, msg, ...) { if(!(check)) { YM##type##ERROR(msg, __VA_ARGS__); YM_DEBUGBREAK(); } }
	#define YM_INTERNAL_VERIFY_WITH_MSG(type, check, ...) YM_INTERNAL_VERIFY_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define YM_INTERNAL_VERIFY_NO_MSG(type, check) YM_INTERNAL_VERIFY_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", YM_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define YM_INTERNAL_VERIFY_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define YM_INTERNAL_VERIFY_GET_MACRO(...) YM_EXPAND_MACRO( YM_INTERNAL_VERIFY_GET_MACRO_NAME(__VA_ARGS__, YM_INTERNAL_VERIFY_WITH_MSG, YM_INTERNAL_VERIFY_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define YM_VERIFY(...) YM_EXPAND_MACRO( YM_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define YM_CORE_VERIFY(...) YM_EXPAND_MACRO( YM_INTERNAL_VERIFY_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define YM_VERIFY(...)
	#define YM_CORE_VERIFY(...)
#endif