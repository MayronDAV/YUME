#include "YUME/yumepch.h"
#include "input.h"
#include "YUME/Events/key_event.h"
#include "YUME/Events/mouse_event.h"



namespace YUME
{
	bool Input::IsKeyPressedOnce(Event& p_Event, int p_KeyCode)
	{
		EventDispatcher dispatch(p_Event);
		bool isPressed = false;
		dispatch.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& p_KeyEvent)
		{
			if (p_KeyEvent.GetKeyCode() == p_KeyCode)
				isPressed = true;

			return isPressed;
		});

		return isPressed;
	}

	bool Input::IsMouseButtonPressedOnce(Event& p_Event, int p_Button)
	{
		EventDispatcher dispatch(p_Event);
		bool isPressed = false;
		dispatch.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& p_ButtonEvent)
		{
			if (p_ButtonEvent.GetMouseButton() == p_Button)
				isPressed = true;

			return isPressed;
		});

		return isPressed;
	}
}