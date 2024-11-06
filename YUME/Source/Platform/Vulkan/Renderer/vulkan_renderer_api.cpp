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

		auto features = VulkanDevice::Get().GetFeatures();
		m_Capabilities.FillModeNonSolid = features.fillModeNonSolid == VK_TRUE;
		m_Capabilities.SamplerAnisotropy = features.samplerAnisotropy == VK_TRUE;
		m_Capabilities.WideLines = features.wideLines == VK_TRUE;
		m_Capabilities.SupportGeometry = features.geometryShader == VK_TRUE;
		m_Capabilities.SupportTesselation = features.tessellationShader == VK_TRUE;
		m_Capabilities.SupportCompute = VulkanDevice::Get().SupportCompute();
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

	void VulkanRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, uint32_t p_Value)
	{
		VkImageSubresourceRange subresourceRange = p_Texture.As<VulkanTexture2D>()->GetSubresourceRange();
		const auto& spec = p_Texture->GetSpecification();
		const auto& commandBuffer = m_Context->GetCommandBuffer();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED || spec.Usage == TextureUsage::TEXTURE_STORAGE)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);

			VkClearColorValue clearColorValue;
			clearColorValue.uint32[0] = p_Value;
			vkCmdClearColorImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_GENERAL, false);
		}
		else
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Unsupported texture usage!")
		}
	}

	void VulkanRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value)
	{
		VkImageSubresourceRange subresourceRange = p_Texture.As<VulkanTexture2D>()->GetSubresourceRange();
		const auto& spec = p_Texture->GetSpecification();
		const auto& commandBuffer = m_Context->GetCommandBuffer();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);

			VkClearColorValue clearColorValue = VkClearColorValue({ { p_Value.x, p_Value.y, p_Value.z, p_Value.w } });
			vkCmdClearColorImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout, false);
		}
		else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, false);

			VkClearDepthStencilValue clear_depth_stencil = { 1.0f, 1 };
			vkCmdClearDepthStencilImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depth_stencil, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout, false);
		}
		else
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Unsupported texture usage!")
		}
	}

	void VulkanRendererAPI::Draw(const Ref<VertexBuffer>& p_VertexBuffer, uint32_t p_VertexCount, uint32_t p_InstanceCount)
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = m_Context->GetCommandBuffer();

		if (p_VertexBuffer != nullptr)
			p_VertexBuffer->Bind();

		vkCmdDraw(commandBuffer, p_VertexCount, p_InstanceCount, 0, 0);
	}

	void VulkanRendererAPI::DrawIndexed(const Ref<VertexBuffer>& p_VertexBuffer, const Ref<IndexBuffer>& p_IndexBuffer, uint32_t p_InstanceCount)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(p_IndexBuffer)

		auto& commandBuffer = m_Context->GetCommandBuffer();

		if (p_VertexBuffer != nullptr)
			p_VertexBuffer->Bind();
		
		p_IndexBuffer->Bind();

		vkCmdDrawIndexed(commandBuffer, p_IndexBuffer->GetCount(), p_InstanceCount, 0, 0, 0);
	}

}
