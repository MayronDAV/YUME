#include "YUME/yumepch.h"
#include "Platform/Vulkan/Renderer/vulkan_renderer_api.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Core/vulkan_swapchain.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"

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
		auto currentFrame = m_Context->GetCurrentFrame();
		auto& commandBuffer = m_Context->GetCommandBuffer();

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
		vkCmdSetViewport(commandBuffer.Get(currentFrame), 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer.Get(currentFrame), 0, 1, &scissor);
	}

	void VulkanRendererAPI::Begin()
	{
		auto currentFrame = m_Context->GetCurrentFrame();
		auto& commandBuffer = m_Context->GetCommandBuffer();

		const auto& renderPass = m_Context->GetRenderPass();
		auto& framebuffer = m_Context->GetSwapchainFramebuffer();

		auto extent = VulkanSwapchain::Get().GetExtent2D();
		renderPass->Begin(commandBuffer.Get(currentFrame), framebuffer.Get(), extent.width, extent.height, m_Color);

		m_Context->HasDrawCommands(true);
	}

	void VulkanRendererAPI::Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		auto currentFrame = m_Context->GetCurrentFrame();
		auto& commandBuffer = m_Context->GetCommandBuffer();

		p_VertexArray->Bind();
		vkCmdDraw(commandBuffer.Get(currentFrame), p_VertexCount, 1, 0, 0);
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount)
	{
		auto currentFrame = m_Context->GetCurrentFrame();
		auto& commandBuffer = m_Context->GetCommandBuffer();

		p_VertexArray->Bind();
		vkCmdDrawIndexed(commandBuffer.Get(currentFrame), p_IndexCount, 1, 0, 0, 0);
	}

	void VulkanRendererAPI::End()
	{
		auto currentFrame = m_Context->GetCurrentFrame();
		auto& commandBuffer = m_Context->GetCommandBuffer();
		m_Context->GetRenderPass()->End(commandBuffer.Get(currentFrame));
	}

}
