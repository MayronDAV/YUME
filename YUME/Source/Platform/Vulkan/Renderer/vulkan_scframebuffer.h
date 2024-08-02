#pragma once
#include "YUME/Core/base.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	// SwapChain Framebuffer
	class YM_API VulkanSCFramebuffer
	{
		public:
			VulkanSCFramebuffer() = default;

			void CleanUp();

			void Init(VkRenderPass p_RenderPass, VkImageView p_ImageView, uint32_t p_Width = 0, uint32_t p_Height = 0);
			void Invalidate(VkImageView p_ImageView, uint32_t p_Width = 0, uint32_t p_Height = 0);

			VkFramebuffer& Get() { return m_Framebuffer; }

		private:
			VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};
}