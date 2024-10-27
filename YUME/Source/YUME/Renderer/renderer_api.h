#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/vertex_array.h"

#include "YUME/Renderer/graphics_context.h"
#include "YUME/Renderer/texture.h"
#include <glm/glm.hpp>


namespace YUME
{
	class YM_API RendererAPI
	{
		public:
			virtual ~RendererAPI() = default;

			virtual void Init(GraphicsContext* p_Context) = 0;

			virtual void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height) = 0;

			virtual void ClearColor(const glm::vec4& p_Color) = 0;
			virtual void ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value) = 0;

			virtual void Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount) = 0;
			virtual void DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount) = 0;

			// Change this later
			static RendererAPI* Create();
	};
}