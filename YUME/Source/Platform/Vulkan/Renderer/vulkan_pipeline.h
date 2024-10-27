#pragma once
#include "YUME/Core/base.h"
#include "YUME/Renderer/shader.h"
#include "YUME/Renderer/vertex_array.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"

#include "YUME/Renderer/renderpass.h"
#include "YUME/Renderer/renderpass_framebuffer.h"

#include <vulkan/vulkan.h>




namespace YUME
{
	class VulkanContext;

	class VulkanPipeline : public Pipeline
	{
		public:
			VulkanPipeline() = default;
			explicit VulkanPipeline(const PipelineCreateInfo& p_Desc);
			~VulkanPipeline() override;

			void CleanUp();

			void Init(const PipelineCreateInfo& p_Desc);

			void Invalidade();
			void Invalidade(const PipelineCreateInfo& p_Desc);

			void Begin() override;
			void End() override;

			void SetPolygonMode(PolygonMode p_Mode) override
			{
				if (m_CreateInfo.PolygonMode != p_Mode)
				{
					m_CreateInfo.PolygonMode = p_Mode;
					Invalidade();
				}
			}

			Shader* GetShader() override { return m_Shader.get(); }
			const VkPipelineLayout& GetLayout() const { return m_Layout; }
			const VkPipeline& GetPipeline() const { return m_Pipeline; }

			void AddVertexArray(const Ref<VertexArray>& p_VertexArray) { m_VertexArrays.push_back(p_VertexArray); Invalidade(); }

		private:
			void TransitionAttachments();
			void CreateFramebuffers();

		private:
			Ref<Shader> m_Shader;

			VulkanContext* m_Context = nullptr;

			Ref<RenderPass> m_RenderPass = nullptr;
			std::vector<Ref<RenderPassFramebuffer>> m_Framebuffers;

			VkPipeline m_Pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_Layout = VK_NULL_HANDLE;

			std::vector<Ref<VertexArray>> m_VertexArrays;
	};
}