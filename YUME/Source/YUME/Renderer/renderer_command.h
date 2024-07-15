#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Renderer/renderer_types.h"



namespace YUME
{

	class YM_PUBLIC RendererCommand
	{
		public:
			static void Init(GraphicsContext* p_Context);
			static void Shutdown();

			static void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height);

			static void Begin();
			static void End();

			static void ClearColor(const glm::vec4& p_Color);

			static void Draw(const RenderPacket& p_Packet);

			static RendererAPI* Get() { return s_RendererAPI; }

		private:
			static RendererAPI* s_RendererAPI;
	};
}