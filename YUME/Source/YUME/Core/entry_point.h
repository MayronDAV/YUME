#pragma once

#include "YUME/Core/application.h"
#include "YUME/Core/log.h"



extern YUME::Application* YUME::CreateApplication();

int main(int p_Argc, char** p_Argv)
{
	YUME::Log::Init();

	auto app = YUME::CreateApplication();
	app->Run();
	delete app;
}