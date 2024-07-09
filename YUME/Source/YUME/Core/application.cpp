#include "YUME/Core/application.h"
#include "YUME/Core/log.h"

#include <iostream>




namespace YUME
{
	Application::Application()
	{
	}

	void Application::Run()
	{
		int i = 0;
		while (true)
		{
			YM_CORE_TRACE("Loops: {}", i)

			if (++i >= 10000)
				break;
		}
	}
}