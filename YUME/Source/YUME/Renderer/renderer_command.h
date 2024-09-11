#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Renderer/renderer_types.h"
#include "YUME/Renderer/vertex_array.h"



namespace YUME
{

	class YM_API RendererCommand
	{
		public:
			static void Init(GraphicsContext* p_Context);
			static void Shutdown();

			static void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height);

			static void Begin();
			static void End();

			static void ClearColor(const glm::vec4& p_Color);

			static void Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount);
			static void DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount);

			static RendererAPI* Get() { return s_RendererAPI; }

		private:
			static RendererAPI* s_RendererAPI;
	};
}