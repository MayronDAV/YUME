#pragma once
#include "YUME/Renderer/renderpass.h"

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

			void Begin(CommandBuffer* p_CommandBuffer, const Ref<Framebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color = { 1, 1, 1, 1 }, SubpassContents p_Contents = SubpassContents::INLINE) override;
			void End(CommandBuffer* p_CommandBuffer) override;

			int GetColorAttachmentCount() const { return m_ColorAttachmentCount; }

			VkRenderPass& Get() { return m_RenderPass; }

		private:
			bool m_ClearEnable = true;
			bool m_ClearDepth = false;
			VkClearValue* m_ClearValue = nullptr;
			int m_ClearCount = 0;
			int m_ColorAttachmentCount = 0;
			bool m_DepthOnly = true;
			std::string m_DebugName;

			VkRenderPass m_RenderPass = nullptr;
	};
}