#pragma once
#include "YUME/Core/base.h"
#include "YUME/Renderer/shader.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"

#include "YUME/Renderer/renderpass.h"
#include "YUME/Renderer/framebuffer.h"

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

			void Begin(CommandBuffer* p_CommandBuffer, SubpassContents p_Contents = SubpassContents::INLINE) override;
			void End(CommandBuffer* p_CommandBuffer) override;

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

			const Ref<RenderPass>& GetRenderPass() const override { return m_RenderPass; }
			const Ref<Framebuffer>& GetFramebuffer() const override;

		private:
			void TransitionAttachments();
			void CreateFramebuffers();
			void ResizeFramebuffer();

		private:
			Ref<Shader> m_Shader;

			VulkanContext* m_Context = nullptr;

			Ref<RenderPass> m_RenderPass = nullptr;
			std::vector<Ref<Framebuffer>> m_Framebuffers;

			VkPipeline m_Pipeline = VK_NULL_HANDLE;
			VkPipelineLayout m_Layout = VK_NULL_HANDLE;
	};
}