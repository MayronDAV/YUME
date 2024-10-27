#include "YUME/yumepch.h"
#include "vulkan_renderpass.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_swapchain.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"
#include "YUME/Core/application.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "YUME/Renderer/texture.h"
#include "Platform/Vulkan/Renderer/vulkan_texture.h"

#include "YUME/Renderer/renderpass_framebuffer.h"
#include "Platform/Vulkan/Renderer/vulkan_renderpass_framebuffer.h"



namespace YUME
{
	namespace Utils
	{
		static VkAttachmentDescription GetAttachmentDesc(const Ref<Texture2D>& p_Texture, bool p_Clear, bool p_SwapchainTarget, int p_Samples = 1)
		{
			YM_PROFILE_FUNCTION()

			VkAttachmentDescription attachmentDesc{};
			const auto& spec = p_Texture->GetSpecification();
			attachmentDesc.format = Utils::TextureFormatToVk(spec.Format);
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = p_Texture.As<VulkanTexture2D>()->GetLayout();

			if (p_SwapchainTarget)
			{
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}

			if (p_Clear)
			{
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				
			}
			else
			{
				if (p_SwapchainTarget)
				{
					attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					attachmentDesc.finalLayout = attachmentDesc.initialLayout;
				}

				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			}

			attachmentDesc.samples = p_Samples > 1 ? (VkSampleCountFlagBits)p_Samples : VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.flags = 0;

			return attachmentDesc;
		}

	} // Utils

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& p_Spec)
		: m_ClearEnable(p_Spec.ClearEnable)
	{
		YM_PROFILE_FUNCTION()

		std::vector<VkAttachmentDescription> attachments;

		std::vector<VkAttachmentReference> colorAttachmentReferences;
		std::vector<VkAttachmentReference> depthAttachmentReferences;
		std::vector<VkSubpassDependency> dependencies;

		for (size_t i = 0; i < p_Spec.Attachments.size(); i++)
		{
			const auto& attachment = p_Spec.Attachments[i];
			const auto& spec = attachment->GetSpecification();

			// attachment desc

			attachments.push_back(Utils::GetAttachmentDesc(attachment, m_ClearEnable, p_Spec.SwapchainTarget));

			// attachment ref

			if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT)
			{
				VkImageLayout layout = attachment.As<VulkanTexture2D>()->GetLayout();
				VkAttachmentReference colorAttachmentRef = {};
				colorAttachmentRef.attachment = uint32_t(i);
				colorAttachmentRef.layout = layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : layout;
				colorAttachmentReferences.push_back(colorAttachmentRef);
				m_DepthOnly = false;
				++m_ColorAttachmentCount;
			}		
			else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
			{
				VkAttachmentReference depthAttachmentRef = {};
				depthAttachmentRef.attachment = uint32_t(i);
				depthAttachmentRef.layout = attachment.As<VulkanTexture2D>()->GetLayout();
				depthAttachmentReferences.push_back(depthAttachmentRef);
				m_ClearDepth = m_ClearEnable;
			}
			else
			{
				YM_CORE_ERROR("Unsupported texture attachment!")
			}

			// Dependencies

			if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
			{
				{
					VkSubpassDependency& depedency = dependencies.emplace_back();
					depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
					depedency.dstSubpass = 0;
					depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					depedency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					depedency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				}

				{
					VkSubpassDependency& depedency = dependencies.emplace_back();
					depedency.srcSubpass = 0;
					depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
					depedency.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					depedency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				}
			}
			else
			{
				{
					VkSubpassDependency& depedency = dependencies.emplace_back();
					depedency.srcSubpass = VK_SUBPASS_EXTERNAL;
					depedency.dstSubpass = 0;
					depedency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					depedency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					depedency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					depedency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				}
				{
					VkSubpassDependency& depedency = dependencies.emplace_back();
					depedency.srcSubpass = 0;
					depedency.dstSubpass = VK_SUBPASS_EXTERNAL;
					depedency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					depedency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					depedency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					depedency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					depedency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				}
			}
		}

		bool resolveTexture = false;
		VkAttachmentReference colorAttachmentResolvedRef = {};

		if (p_Spec.ResolveTexture != nullptr && p_Spec.Samples > 1)
		{
			resolveTexture = true;
			VkImageLayout layout = p_Spec.ResolveTexture.As<VulkanTexture2D>()->GetLayout();
			colorAttachmentResolvedRef.attachment = uint32_t(attachments.size());
			colorAttachmentResolvedRef.layout = layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : layout;

			attachments.push_back(Utils::GetAttachmentDesc(p_Spec.ResolveTexture, m_ClearEnable, p_Spec.SwapchainTarget));
		}


		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
		subpass.pColorAttachments = colorAttachmentReferences.data();
		subpass.pDepthStencilAttachment = depthAttachmentReferences.data();
		subpass.pResolveAttachments = resolveTexture ? &colorAttachmentResolvedRef : VK_NULL_HANDLE;

		m_ColorAttachmentCount = int(colorAttachmentReferences.size());

		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpass;
		//renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		//renderPassCreateInfo.pDependencies = dependencies.data();
		renderPassCreateInfo.dependencyCount = 0;
		renderPassCreateInfo.pDependencies = nullptr;

		auto res = vkCreateRenderPass(VulkanDevice::Get().GetDevice(), &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		m_ClearValue = new VkClearValue[int(attachments.size())];
		m_ClearCount = int(attachments.size());
	}

	VulkanRenderPass::~VulkanRenderPass()
	{
		YM_PROFILE_FUNCTION()

		CleanUp(true);
	}

	void VulkanRenderPass::CleanUp(bool p_DeletionQueue) noexcept
	{
		if (!p_DeletionQueue)
		{
			if (m_RenderPass != VK_NULL_HANDLE)
			{
				YM_CORE_TRACE("Destroying vulkan renderpass")
				vkDestroyRenderPass(VulkanDevice::Get().GetDevice(), m_RenderPass, VK_NULL_HANDLE);
			}
		}
		else
		{
			auto renderpass = m_RenderPass;
			VulkanContext::PushFunction([renderpass]()
			{
				if (renderpass != VK_NULL_HANDLE)
				{
					YM_CORE_TRACE("Destroying vulkan renderpass")
					vkDestroyRenderPass(VulkanDevice::Get().GetDevice(), renderpass, VK_NULL_HANDLE);
				}
			});
		}
	}

	void VulkanRenderPass::Begin(const Ref<RenderPassFramebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_ASSERT(p_Frame != nullptr)
		auto framebuffer = p_Frame.As<VulkanRenderPassFramebuffer>()->Get();
		YM_CORE_ASSERT(framebuffer != VK_NULL_HANDLE)
		YM_CORE_ASSERT(m_ClearValue != nullptr)

		if (!m_DepthOnly)
		{
			for (int i = 0; i < m_ClearCount; i++)
			{
				m_ClearValue[i].color.float32[0] = p_Color.r;
				m_ClearValue[i].color.float32[1] = p_Color.g;
				m_ClearValue[i].color.float32[2] = p_Color.b;
				m_ClearValue[i].color.float32[3] = p_Color.a;
			}
		}

		if (m_ClearDepth)
		{
			m_ClearValue[m_ClearCount - 1].depthStencil = VkClearDepthStencilValue{ 1.0f, 0 };
		}

		VkRenderPassBeginInfo rpBegin = {};
		rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBegin.pNext = VK_NULL_HANDLE;
		rpBegin.renderPass = m_RenderPass;
		rpBegin.framebuffer = framebuffer;
		rpBegin.renderArea.offset.x = 0;
		rpBegin.renderArea.offset.y = 0;
		rpBegin.renderArea.extent.width = p_Width;
		rpBegin.renderArea.extent.height = p_Height;
		rpBegin.clearValueCount = (m_ClearEnable) ? m_ClearCount : 0;
		rpBegin.pClearValues = m_ClearValue;

		auto& commandBuffer = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext())->GetCommandBuffer();
		vkCmdBeginRenderPass(commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderPass::End()
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext())->GetCommandBuffer();
		vkCmdEndRenderPass(commandBuffer);
	}

} // YUME
