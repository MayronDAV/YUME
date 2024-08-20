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
	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& p_Spec)
		: m_ClearEnable(p_Spec.ClearEnable)
	{
		YM_PROFILE_FUNCTION()

		std::vector<VkAttachmentDescription> attachmentDescs;

		for (const auto& attachment : p_Spec.Attachments)
		{
			auto spec = attachment->GetSpecification();

			VkAttachmentDescription attachmentDesc{};

			if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT)
			{
				attachmentDesc.format = Utils::TextureFormatToVk(spec.Format);
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				if (m_ClearEnable)
				{
					attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
					attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				}
				else
				{
					attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				}

				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}
			else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
			{
				YM_CORE_VERIFY(false, "NOT IMPLEMENTED!")
			}
			else
			{
				YM_CORE_CRITICAL("Unsupported texture usage!")
				continue;
			}

			attachmentDescs.push_back(attachmentDesc);
		}

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)attachmentDescs.size();
		renderPassInfo.pAttachments = attachmentDescs.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		auto res = vkCreateRenderPass(VulkanDevice::Get().GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);
		YM_CORE_VERIFY(res == VK_SUCCESS)
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

	void VulkanRenderPass::Begin(const Ref<RenderPassFramebuffer>& p_Frame)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(p_Frame != nullptr)
		auto framebuffer = std::dynamic_pointer_cast<VulkanRenderPassFramebuffer>(p_Frame)->Get();
		YM_CORE_VERIFY(framebuffer != VK_NULL_HANDLE)

		VkClearValue clearValue;
		clearValue.color.float32[0] = m_ClearColor.r;
		clearValue.color.float32[1] = m_ClearColor.g;
		clearValue.color.float32[2] = m_ClearColor.b;
		clearValue.color.float32[3] = m_ClearColor.a;

		if (m_ClearDepth)
		{
			clearValue.depthStencil = VkClearDepthStencilValue{ 1.0f, 0 };
		}

		VkRenderPassBeginInfo rpBegin = {};
		rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBegin.pNext = VK_NULL_HANDLE;
		rpBegin.renderPass = m_RenderPass;
		rpBegin.framebuffer = framebuffer;
		rpBegin.renderArea.offset.x = 0;
		rpBegin.renderArea.offset.y = 0;
		rpBegin.renderArea.extent.width = m_Width;
		rpBegin.renderArea.extent.height = m_Height;
		if (m_ClearEnable)
		{
			rpBegin.clearValueCount = 1;
			rpBegin.pClearValues = &clearValue;
		}
		else
		{
			rpBegin.clearValueCount = 0;
			rpBegin.pClearValues = nullptr;
		}

		auto& commandBuffer = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext())->GetCommandBuffer();
		vkCmdBeginRenderPass(commandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderPass::End()
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext())->GetCommandBuffer();
		vkCmdEndRenderPass(commandBuffer);
	}
}
