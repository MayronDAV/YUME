#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/engine.h"
#include "YUME/Core/command_buffer.h"



namespace YUME
{
	class YM_API GraphicsContext
	{
		public:
			virtual ~GraphicsContext() = default;

			virtual void Init(const char* p_Name, void* p_Window) = 0;

			virtual void OnResize(uint32_t p_Width, uint32_t p_Height) {};

			virtual CommandBuffer* GetCurrentCommandBuffer() = 0;

			virtual void Begin() {}
			virtual void End() = 0;

			virtual void SwapBuffer() = 0;
	};
}