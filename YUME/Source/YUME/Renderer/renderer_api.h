#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/renderer_types.h"

#include "YUME/Renderer/graphics_context.h"
#include <glm/glm.hpp>


namespace YUME
{
	class YM_API RendererAPI
	{
		public:
			virtual ~RendererAPI() = default;

			virtual void Init(GraphicsContext* p_Context) = 0;

			virtual void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height) = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			virtual void ClearColor(const glm::vec4& p_Color) = 0;

			virtual void Draw(const RenderPacket& p_Packet) = 0;

			// Change this later
			static RendererAPI* Create();
	};
}