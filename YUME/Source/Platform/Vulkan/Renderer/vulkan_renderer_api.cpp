#include "YUME/yumepch.h"
#include "Platform/Vulkan/Renderer/vulkan_renderer_api.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"

#include "YUME/Core/application.h"

#include <typeindex>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan_texture.h"



namespace YUME
{
	VulkanRendererAPI::~VulkanRendererAPI() = default;

	void VulkanRendererAPI::Init(GraphicsContext* p_Context)
	{
		m_Context = dynamic_cast<VulkanContext*>(p_Context);
	}

	void VulkanRendererAPI::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		YM_PROFILE_FUNCTION()

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
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.baseMipLevel = 0;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;

		const auto& spec = p_Texture->GetSpecification();
		const auto& commandBuffer = m_Context->GetCommandBuffer();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);

			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

			VkClearColorValue clearColorValue = VkClearColorValue({ { p_Value.x, p_Value.y, p_Value.z, p_Value.w } });
			vkCmdClearColorImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout, false);
		}
		else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);

			subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			VkClearDepthStencilValue clear_depth_stencil = { 1.0f, 1 };
			vkCmdClearDepthStencilImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depth_stencil, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout, false);
		}
		else
		{
			YM_CORE_ERROR("Unsupported texture usage!")
		}

	}

	void VulkanRendererAPI::Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = m_Context->GetCommandBuffer();

		if (p_VertexArray != nullptr)
			p_VertexArray->Bind();
		vkCmdDraw(commandBuffer, p_VertexCount, 1, 0, 0);
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount)
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = m_Context->GetCommandBuffer();

		p_VertexArray->Bind();
		vkCmdDrawIndexed(commandBuffer, p_IndexCount, 1, 0, 0, 0);
	}

}
