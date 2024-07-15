#include "YUME/yumepch.h"
#include "YUME/Core/input.h"
#include "YUME/Core/application.h"


#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace YUME
{
	bool Input::IsKeyPressed(int p_Keycode)
	{
		auto glfwWindow = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetKey(glfwWindow, p_Keycode);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonPressed(int p_Button)
	{
		auto glfwWindow = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(glfwWindow, p_Button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto glfwWindow = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(glfwWindow, &xpos, &ypos);

		return { (float)xpos, (float)ypos };

	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}

}