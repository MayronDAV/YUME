#pragma once
#include "YUME/Renderer/renderpass.h"
#include "Platform/Vulkan/Renderer/vulkan_vertex_array.h"

// Lib
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>



namespace YUME
{
	class VulkanRenderPass : public RenderPass
	{
		public:
			VulkanRenderPass() = default;
			explicit VulkanRenderPass(const RenderPassSpecification& p_Spec);
			~VulkanRenderPass() override;

			void CleanUp(bool p_DeletionQueue = false) noexcept;

			void Begin(const Ref<RenderPassFramebuffer>& p_Frame) override;
			void End() override;

			void SetClearColor(const glm::vec4& p_Color) override { m_ClearColor = p_Color; }
			void EnableClearDepth(bool p_Enable) override { m_ClearEnable = p_Enable; }
			void SetViewport(uint32_t p_Width, uint32_t p_Height) override { m_Width = p_Width; m_Height = p_Height; }

			void SetCurrentFrame(VkFramebuffer& p_Frame) { m_CurrentFrame = p_Frame; }

			VkRenderPass& Get() { return m_RenderPass; }

		private:
			uint32_t m_Width = 0;
			uint32_t m_Height = 0;
			glm::vec4 m_ClearColor = { 1, 1, 1, 1 };
			bool m_ClearEnable = true;
			bool m_ClearDepth = false;
			VkFramebuffer m_CurrentFrame = VK_NULL_HANDLE;

			VkRenderPass m_RenderPass = nullptr;
	};
}