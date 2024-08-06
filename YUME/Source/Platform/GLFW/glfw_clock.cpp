#include "YUME/yumepch.h"
#include "YUME/Utils/clock.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


namespace YUME
{
	double Clock::GetTime()
	{
		auto time = glfwGetTime();
		if (time == 0)
		{
			YM_CORE_ERROR("Something wrong, did you call glfwInit?")
			return 0;
		}
		return time;
	}
}