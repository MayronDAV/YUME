#pragma once

#include "YUME/Core/base.h"
#include "YUME/Events/key_event.h"


namespace YUME
{
	class YM_API Input
	{
		public:
			static bool IsKeyPressed(int p_Keycode);
			// This way the key event does not propagate to different layers.
			static bool IsKeyPressedOnce(Event& p_Event, int p_KeyCode);

			static bool IsMouseButtonPressed(int p_Button);
			// This way the mouse button event does not propagate to different layers.
			static bool IsMouseButtonPressedOnce(Event& p_Event, int p_Button);

			static std::pair<float, float> GetMousePosition();
			static float GetMouseX();
			static float GetMouseY();
	};
}