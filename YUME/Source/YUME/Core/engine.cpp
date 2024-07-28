#include "YUME/yumepch.h"
#include "engine.h"

namespace YUME
{
	Engine* Engine::s_Instance = nullptr;

	void Engine::Init()
	{
		if (s_Instance != nullptr)
			delete s_Instance;

		s_Instance = new Engine();
	}

	void Engine::Release()
	{
		if (s_Instance != nullptr)
			delete s_Instance;
	}
}
