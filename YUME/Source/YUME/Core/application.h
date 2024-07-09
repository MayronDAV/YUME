#pragma once

#include "YUME/Core/base.h"


namespace YUME
{
	class YM_PUBLIC Application
	{
		public:
			Application();
			virtual ~Application() = default;

			void Run();
	};

	Application* CreateApplication();
}

