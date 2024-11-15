#pragma once
#include "YUME/Renderer/framebuffer.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class VulkanFramebuffer : public Framebuffer
	{
		public:
			VulkanFramebuffer() = default;
			explicit VulkanFramebuffer(const FramebufferSpecification& p_Spec);
			~VulkanFramebuffer() override;

			void CleanUp(bool p_DeletionQueue = false);

			void OnResize(uint32_t p_Width, uint32_t p_Height, const std::vector<Ref<Texture2D>>& p_Attachments = {}) override;
			uint32_t GetWidth() const override { return m_Spec.Width; }
			uint32_t GetHeight() const override { return m_Spec.Height; }

			VkFramebuffer& Get() { return m_Framebuffer; }

		private:
			void Init(const FramebufferSpecification& p_Spec = {});

		private:
			FramebufferSpecification m_Spec = {};
			VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
	};
}