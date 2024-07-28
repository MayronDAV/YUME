#include "YUME/yumepch.h"
#include "vulkan_scframebuffer.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"



namespace YUME
{
	void VulkanSCFramebuffer::CleanUp()
	{
		if (m_Framebuffer != VK_NULL_HANDLE)
		{
			YM_CORE_TRACE("Destroying vulkan swapchain framebuffer...")
			vkDestroyFramebuffer(VulkanDevice::Get().GetDevice(), m_Framebuffer, VK_NULL_HANDLE);
		}
	}

	void VulkanSCFramebuffer::Init(VkRenderPass p_RenderPass, VkImageView p_ImageView, uint32_t p_Width, uint32_t p_Height)
	{
		m_RenderPass = p_RenderPass;

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &p_ImageView;
		
		if (p_Width == 0 || p_Height == 0)
		{
			auto extent = VulkanSwapchain::Get().GetExtent2D();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
		}
		else
		{
			framebufferInfo.width = p_Width;
			framebufferInfo.height = p_Height;
		}

		framebufferInfo.layers = 1;

		auto res = vkCreateFramebuffer(VulkanDevice::Get().GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

	void VulkanSCFramebuffer::Invalidate(VkImageView p_ImageView, uint32_t p_Width, uint32_t p_Height)
	{
		CleanUp();

		Init(m_RenderPass, p_ImageView, p_Width, p_Height);
	}
}
