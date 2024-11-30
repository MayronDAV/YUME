#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Renderer/buffer.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Renderer/mesh.h"
#include "YUME/Core/command_buffer.h"

#include <glm/glm.hpp>



namespace YUME
{

	class YM_API RendererCommand
	{
		public:
			static void Init(GraphicsContext* p_Context);
			static void Shutdown();

			static void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height, CommandBuffer* p_CommandBuffer = nullptr);

			static void ClearColor(const glm::vec4& p_Color);
			static void ClearRenderTarget(const Ref<Texture2D> p_Texture, uint32_t p_Value);
			static void ClearRenderTarget(const Ref<Texture2D> p_Texture, const glm::vec4& p_Value = { 0.0f, 0.0f, 0.0f, 1.0f });

			static void Draw(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, uint32_t p_VertexCount, uint32_t p_InstanceCount = 1);
			static void DrawIndexed(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, const Ref<IndexBuffer>& p_IndexBuffer, uint32_t p_InstanceCount = 1);
			static void DrawMesh(CommandBuffer* p_CommandBuffer, const Ref<Mesh>& p_Mesh);

			static void SaveScreenshot(const std::string& p_OutPath, const Ref<Texture>& p_Texture, bool p_Async = true);

			static const Capabilities& GetCapabilities();

			static void BindDescriptorSets(CommandBuffer* p_CommandBuffer, const Ref<DescriptorSet>* p_DescriptorSets, uint32_t p_Count = 1);

			static RendererAPI* Get() { return s_RendererAPI; }

		private:
			static RendererAPI* s_RendererAPI;
	};
}