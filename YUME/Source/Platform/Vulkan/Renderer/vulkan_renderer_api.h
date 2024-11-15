#pragma once

#include "YUME/Renderer/renderer_api.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"

#include <vulkan/vulkan.h>


namespace YUME
{
	class VulkanRendererAPI : public RendererAPI
	{
		public:
			VulkanRendererAPI() = default;
			~VulkanRendererAPI() override;

			void Init(GraphicsContext* p_Context) override;

			void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height, CommandBuffer* p_CommandBuffer = nullptr) override;

			void ClearColor(const glm::vec4& p_Color) override { m_Color = p_Color; }
			void ClearRenderTarget(const Ref<Texture2D>& p_Texture, uint32_t p_Value) override;
			void ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value = { 0.0f, 0.0f, 0.0f, 1.0f }) override;

			const Capabilities& GetCapabilities() const override { return m_Capabilities; }

			void Draw(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, uint32_t p_VertexCount, uint32_t p_InstanceCount = 1) override;
			void DrawIndexed(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, const Ref<IndexBuffer>& p_IndexBuffer, uint32_t p_InstanceCount = 1) override;
		private:
			VulkanContext* m_Context = nullptr;
			Capabilities m_Capabilities;

			glm::vec4 m_Color = {0.0f, 0.0f, 0.0f, 1.0f};
	};
}