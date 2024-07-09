#pragma once

#include "YUME/Core/base.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <string>
#include <string_view>
#include <memory>



namespace YUME
{

	class YM_PUBLIC Log
	{
		public:

			static void Init();

			inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
			inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

		private:
			static std::shared_ptr<spdlog::logger> s_CoreLogger;
			static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

#ifndef YM_DIST
	// Core log macros
	#define YM_CORE_TRACE(...)		::YUME::Log::GetCoreLogger()->trace(__VA_ARGS__);
	#define YM_CORE_INFO(...)    	::YUME::Log::GetCoreLogger()->info(__VA_ARGS__);
	#define YM_CORE_WARN(...)    	::YUME::Log::GetCoreLogger()->warn(__VA_ARGS__);
	#define YM_CORE_ERROR(...)   	::YUME::Log::GetCoreLogger()->error(__VA_ARGS__);
	#define YM_CORE_CRITICAL(...)	::YUME::Log::GetCoreLogger()->critical(__VA_ARGS__);

	// Client log macros
	#define YM_TRACE(...)			::YUME::Log::GetClientLogger()->trace(__VA_ARGS__);
	#define YM_INFO(...)	      	::YUME::Log::GetClientLogger()->info(__VA_ARGS__);
	#define YM_WARN(...)			::YUME::Log::GetClientLogger()->warn(__VA_ARGS__);
	#define YM_ERROR(...)			::YUME::Log::GetClientLogger()->error(__VA_ARGS__);
	#define YM_CRITICAL(...)		::YUME::Log::GetClientLogger()->critical(__VA_ARGS__);
#else
	// Core log macros
	#define YM_CORE_TRACE(...)
	#define YM_CORE_INFO(...)
	#define YM_CORE_WARN(...)
	#define YM_CORE_ERROR(...)
	#define YM_CORE_CRITICAL(...)

	// Client log macros
	#define YM_TRACE(...)
	#define YM_INFO(...)
	#define YM_WARN(...)
	#define YM_ERROR(...)
	#define YM_CRITICAL(...)
#endif