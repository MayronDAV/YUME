#pragma once

#include "YUME/Renderer/renderer_api.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"

#include <vulkan/vulkan.h>


namespace YUME
{
	class YM_API VulkanRendererAPI : public RendererAPI
	{
		public:
			VulkanRendererAPI() = default;
			~VulkanRendererAPI() override;

			void Init(GraphicsContext* p_Context) override;

			void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height) override;

			void Begin() override;
			void End() override;

			void ClearColor(const glm::vec4& p_Color) override { m_Color = p_Color; }

			void Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount) override;
			void DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount) override;

		private:
			VulkanContext* m_Context = nullptr;

			glm::vec4 m_Color = {0.0f, 0.0f, 0.0f, 1.0f};
	};
}