#pragma once

#include "YUME/Core/application.h"
#include "YUME/Core/log.h"

#include <iostream>

#if defined(YM_DIST)
	#if defined(YM_PLATFORM_WINDOWS)
		#include <Windows.h>

		void DisableConsole()
		{
			HWND hWnd = GetConsoleWindow();
			if (hWnd != nullptr)
				ShowWindow(hWnd, SW_HIDE);
		}
	#elif defined(YM_PLATFORM_LINUX)
		#include <fstream>

		void DisableConsole()
		{
			std::ofstream nullout("/dev/null");
			std::cout.rdbuf(nullout.rdbuf());
			std::cerr.rdbuf(nullout.rdbuf());
		}
	#endif
#else
	#if defined(YM_PLATFORM_WINDOWS)
		void EnableConsole()
		{
			HWND hWnd = GetConsoleWindow();
			if (hWnd != nullptr)
				ShowWindow(hWnd, SW_SHOW);
		}
	#elif defined(YM_PLATFORM_LINUX)
		void EnableConsole()
		{
			std::cout.rdbuf(std::cout.rdbuf());
			std::cerr.rdbuf(std::cerr.rdbuf());
		}
	#endif
#endif


extern YUME::Application* YUME::CreateApplication();

int main(int p_Argc, char** p_Argv)
{
	#ifdef YM_DIST
		DisableConsole();
	#else
		EnableConsole();
	#endif

	YUME::Log::Init();

	auto app = YUME::CreateApplication();
	app->Run();
	delete app;
}