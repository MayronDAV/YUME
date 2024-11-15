#include "YUME/yumepch.h"
#include "vulkan_framebuffer.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "YUME/Renderer/texture.h"
#include "vulkan_texture.h"
#include "vulkan_context.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"



namespace YUME
{
	VulkanFramebuffer::VulkanFramebuffer(const FramebufferSpecification& p_Spec)
		: m_Spec(p_Spec)
	{
		Init(p_Spec);
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		CleanUp(true);
	}

	void VulkanFramebuffer::CleanUp(bool p_DeletionQueue)
	{
		YM_PROFILE_FUNCTION()

		if (!p_DeletionQueue)
		{
			if (m_Framebuffer != VK_NULL_HANDLE)
			{
				YM_CORE_TRACE(VULKAN_PREFIX "Destroying framebuffer...")
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
					YM_CORE_TRACE(VULKAN_PREFIX "Destroying framebuffer...")
					vkDestroyFramebuffer(VulkanDevice::Get().GetDevice(), framebuffer, VK_NULL_HANDLE);
				}
			});
		}
	}

	void VulkanFramebuffer::OnResize(uint32_t p_Width, uint32_t p_Height, const std::vector<Ref<Texture2D>>& p_Attachments)
	{
		YM_PROFILE_FUNCTION()

		m_Spec.Width = p_Width;
		m_Spec.Height = p_Height;

		if (p_Attachments.empty())
		{
			for (auto& attachment : m_Spec.Attachments)
			{
				attachment->Resize(p_Width, p_Height);
			}
		}
		else
		{
			m_Spec.Attachments = p_Attachments;
		}

		CleanUp();

		Init(m_Spec);
	}

	void VulkanFramebuffer::Init(const FramebufferSpecification& p_Spec)
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

		if (!m_Spec.DebugName.empty())
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_FRAMEBUFFER, m_Spec.DebugName.c_str(), m_Framebuffer);
	}
}
