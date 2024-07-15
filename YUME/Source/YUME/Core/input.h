#pragma once

#include "YUME/Core/base.h"


namespace YUME
{
	class YM_PUBLIC Input
	{
		public:
			static bool IsKeyPressed(int p_Keycode);

			static bool IsMouseButtonPressed(int p_Button);
			static std::pair<float, float> GetMousePosition();
			static float GetMouseX();
			static float GetMouseY();
	};
}