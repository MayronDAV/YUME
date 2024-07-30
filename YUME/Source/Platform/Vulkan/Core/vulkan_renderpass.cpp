#include "YUME/yumepch.h"
#include "vulkan_renderpass.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"



namespace YUME
{
	VulkanRenderPass::~VulkanRenderPass()
	{
		if (m_RenderPass != VK_NULL_HANDLE)
		{
			YM_CORE_TRACE("Destroying vulkan renderpass")
			vkDestroyRenderPass(VulkanDevice::Get().GetDevice(), m_RenderPass, VK_NULL_HANDLE);
		}
	}

	void VulkanRenderPass::Init(bool p_ClearEnable)
	{
		// TODO: Make it more configurable.

		m_ClearEnable = p_ClearEnable;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VulkanSwapchain::Get().GetFormat().format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (p_ClearEnable)
		{
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		}
		else
		{
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		auto res = vkCreateRenderPass(VulkanDevice::Get().GetDevice(), &renderPassInfo, nullptr, &m_RenderPass);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

	void VulkanRenderPass::Begin(VkCommandBuffer p_CommandBuffer, VkFramebuffer p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color, bool p_ClearDepth)
	{
		VkClearValue clearValue;
		clearValue.color.float32[0] = p_Color.r;
		clearValue.color.float32[1] = p_Color.g;
		clearValue.color.float32[2] = p_Color.b;
		clearValue.color.float32[3] = p_Color.a;

		if (p_ClearDepth)
		{
			clearValue.depthStencil = VkClearDepthStencilValue{ 1.0f, 0 };
		}

		VkRenderPassBeginInfo rpBegin = {};
		rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBegin.pNext = VK_NULL_HANDLE;
		rpBegin.renderPass = m_RenderPass;
		rpBegin.framebuffer = p_Frame;
		rpBegin.renderArea.offset.x = 0;
		rpBegin.renderArea.offset.y = 0;
		rpBegin.renderArea.extent.width = p_Width;
		rpBegin.renderArea.extent.height = p_Height;
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

		vkCmdBeginRenderPass(p_CommandBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderPass::End(VkCommandBuffer p_CommandBuffer)
	{
		vkCmdEndRenderPass(p_CommandBuffer);
	}
}
