#include "YUME/yumepch.h"
#include "Platforms/Vulkan/Renderer/vulkan_renderer_api.h"
#include "Platforms/Vulkan/Utils/vulkan_utils.h"

#include "YUME/Core/application.h"

#include <typeindex>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace YUME
{


	VulkanRendererAPI::~VulkanRendererAPI() = default;

	void VulkanRendererAPI::Init(GraphicsContext* p_Context)
	{
		m_Context = dynamic_cast<VulkanContext*>(p_Context);
	}

	void VulkanRendererAPI::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		VkViewport viewport{};
		viewport.x = p_X;
		viewport.y = p_Y;
		VkExtent2D extent = {
			p_Width, p_Height
		};
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_Context->GetCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(m_Context->GetCommandBuffer(), 0, 1, &scissor);
	}

	void VulkanRendererAPI::Begin()
	{
		m_Context->TransitionImageLayout(m_Context->GetImage(), VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_Context->BeginFrame();
		auto& commandBuffer = m_Context->GetCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		auto res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		YM_CORE_ASSERT(res == VK_SUCCESS)

		auto& renderPass = m_Context->GetRenderPass();
		auto& framebuffer = m_Context->GetSwapchainFramebuffer();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_Context->GetCurrentExtent2D();

		VkClearValue clearColor = { {{m_Color.r, m_Color.g, m_Color.b, m_Color.a}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		m_Context->HasDrawCommands(true);
	}


	void VulkanRendererAPI::Draw(const RenderPacket& p_Packet)
	{
	}

	void VulkanRendererAPI::End()
	{
		m_Context->TransitionImageLayout(m_Context->GetImage(), VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		auto& commandBuffer = m_Context->GetCommandBuffer();
		vkCmdEndRenderPass(commandBuffer);

		auto res = vkEndCommandBuffer(commandBuffer);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		VkSemaphore waitSemaphores[] = { m_Context->GetSignalSemaphore() };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkCommandBuffer commandBuffers[] = { commandBuffer };
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = commandBuffers;

		submitInfo.signalSemaphoreCount = 1;
		VkSemaphore signalSemaphores[] = { m_Context->GetWaitSemaphore() };
		submitInfo.pSignalSemaphores = signalSemaphores;

		res = vkQueueSubmit(m_Context->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

}
