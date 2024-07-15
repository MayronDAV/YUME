#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/yume_config.h"


namespace YUME
{
	class YM_API GraphicsContext
	{
		public:
			virtual ~GraphicsContext() = default;

			virtual void Init(void* p_Window) = 0;

			virtual void SwapBuffer() = 0;
	};
}