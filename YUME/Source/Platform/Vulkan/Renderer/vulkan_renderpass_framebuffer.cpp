#include "YUME/yumepch.h"
#include "vulkan_renderpass_framebuffer.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "YUME/Renderer/texture.h"
#include "vulkan_texture.h"
#include "vulkan_context.h"



namespace YUME
{
	VulkanRenderPassFramebuffer::VulkanRenderPassFramebuffer(const RenderPassFramebufferSpec& p_Spec)
		: m_Spec(p_Spec)
	{
		Init(p_Spec);
	}

	VulkanRenderPassFramebuffer::~VulkanRenderPassFramebuffer()
	{
		CleanUp(true);
	}

	void VulkanRenderPassFramebuffer::CleanUp(bool p_DeletionQueue)
	{
		YM_PROFILE_FUNCTION()

		if (!p_DeletionQueue)
		{
			if (m_Framebuffer != VK_NULL_HANDLE)
			{
				YM_CORE_TRACE("Destroying vulkan swapchain framebuffer...")
				vkDestroyFramebuffer(VulkanDevice::Get().GetDevice(), m_Framebuffer, VK_NULL_HANDLE);
			}
		}
		else
		{
			auto framebuffer = m_Framebuffer;
			VulkanContext::PushFunction([framebuffer]()
			{
				if (framebuffer != VK_NULL_HANDLE)
				{
					YM_CORE_TRACE("Destroying vulkan swapchain framebuffer...")
					vkDestroyFramebuffer(VulkanDevice::Get().GetDevice(), framebuffer, VK_NULL_HANDLE);
				}
			});
		}
	}

	void VulkanRenderPassFramebuffer::OnResize(uint32_t p_Width, uint32_t p_Height, const std::vector<Ref<Texture2D>>& p_Attachments)
	{
		YM_PROFILE_FUNCTION()

		m_Spec.Width = p_Width;
		m_Spec.Height = p_Height;
		m_Spec.Attachments = p_Attachments;

		CleanUp();

		Init(m_Spec);
	}

	void VulkanRenderPassFramebuffer::Init(const RenderPassFramebufferSpec& p_Spec)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_Spec.RenderPass.As<VulkanRenderPass>()->Get();
		framebufferInfo.attachmentCount = (uint32_t)m_Spec.Attachments.size();

		std::vector<VkImageView> attachments;
		for (const auto& attachment : m_Spec.Attachments)
		{
			attachments.push_back(attachment.As<VulkanTexture2D>()->GetImageView());
		}
		framebufferInfo.pAttachments = attachments.data();

		if (m_Spec.Width == 0 || m_Spec.Height == 0)
		{
			auto extent = VulkanSwapchain::Get().GetExtent2D();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
		}
		else
		{
			framebufferInfo.width = m_Spec.Width;
			framebufferInfo.height = m_Spec.Height;
		}

		framebufferInfo.layers = 1;

		auto res = vkCreateFramebuffer(VulkanDevice::Get().GetDevice(), &framebufferInfo, nullptr, &m_Framebuffer);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}
}
