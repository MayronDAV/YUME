#include "YUME/yumepch.h"
#include "YUME/Core/log.h"
#include "YUME/Events/event.h"

#include <spdlog/sinks/stdout_color_sinks.h>




namespace YUME
{

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		//spdlog::set_pattern("%n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("YUME");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}


}