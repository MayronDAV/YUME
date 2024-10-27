#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Renderer/vertex_array.h"
#include "YUME/Renderer/texture.h"

#include <glm/glm.hpp>



namespace YUME
{

	class YM_API RendererCommand
	{
		public:
			static void Init(GraphicsContext* p_Context);
			static void Shutdown();

			static void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height);

			static void ClearColor(const glm::vec4& p_Color);
			static void ClearRenderTarget(const Ref<Texture2D> p_Texture, const glm::vec4& p_Value);

			static void Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount);
			static void DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount);

			static RendererAPI* Get() { return s_RendererAPI; }

		private:
			static RendererAPI* s_RendererAPI;
	};
}