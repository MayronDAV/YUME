#pragma once

#ifdef YM_PLATFORM_WINDOWS
	#ifdef YM_BUILD_DLL
		#define YM_PUBLIC __declspec(dllexport)
	#else
		#define YM_PUBLIC __declspec(dllimport)
	#endif
	#define YM_LOCAL
#else
	#if YM_PLATFORM_LINUX
		#ifdef YM_BUILD_DLL
			#define YM_PUBLIC __attribute__((visibility("default")))
			#define YM_LOCAL  __attribute__((visibility("hidden")))
		#else
			#define YM_PUBLIC
			#define YM_LOCAL
		#endif
	#else
		#error Currently only support windows and linux!
	#endif
#endif