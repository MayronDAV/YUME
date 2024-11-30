#include "YUME/yumepch.h"
#include "Platform/Vulkan/Renderer/vulkan_renderer_api.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "vulkan_texture.h"
#include "YUME/Core/application.h"


#include <typeindex>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>



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

	void VulkanRendererAPI::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height, CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = p_CommandBuffer ? static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle() : VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer->GetHandle();

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
		auto subresourceRange = p_Texture.As<VulkanTexture2D>()->GetSubresourceRange();
		const auto& spec	  = p_Texture->GetSpecification();
		auto& frame			  = VulkanSwapchain::Get().GetCurrentFrameData();
		auto& commandBuffer	  = frame.MainCommandBuffer->GetHandle();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED || spec.Usage == TextureUsage::TEXTURE_STORAGE)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, frame.MainCommandBuffer.get());

			VkClearColorValue clearColorValue;
			clearColorValue.uint32[0] = p_Value;
			vkCmdClearColorImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_GENERAL, frame.MainCommandBuffer.get());
		}
		else
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Unsupported texture usage!")
		}
	}

	void VulkanRendererAPI::ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value)
	{
		auto subresourceRange = p_Texture.As<VulkanTexture2D>()->GetSubresourceRange();
		const auto& spec	  = p_Texture->GetSpecification();
		auto& frame			  = VulkanSwapchain::Get().GetCurrentFrameData();
		auto& commandBuffer   = frame.MainCommandBuffer->GetHandle();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, frame.MainCommandBuffer.get());

			VkClearColorValue clearColorValue = VkClearColorValue({ { p_Value.x, p_Value.y, p_Value.z, p_Value.w } });
			vkCmdClearColorImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorValue, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout, frame.MainCommandBuffer.get());
		}
		else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
		{
			VkImageLayout layout = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, frame.MainCommandBuffer.get());

			VkClearDepthStencilValue clear_depth_stencil = { 1.0f, 1 };
			vkCmdClearDepthStencilImage(commandBuffer, p_Texture.As<VulkanTexture2D>()->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depth_stencil, 1, &subresourceRange);
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout, frame.MainCommandBuffer.get());
		}
		else
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Unsupported texture usage!")
		}
	}

	void VulkanRendererAPI::SaveScreenshot(const std::string& p_OutPath, const Ref<Texture>& p_Texture, bool p_Async)
	{
		YM_PROFILE_FUNCTION()

		// TODO: Make it work for texture arrays!

		auto& device					  = VulkanDevice::Get().GetDevice();
		auto& physDevice				  = VulkanDevice::Get().GetPhysicalDevice();

		VkFormatProperties formatProps;
		auto format						  = VKUtils::TextureFormatToVk(p_Texture->GetSpecification().Format);

		vkGetPhysicalDeviceFormatProperties(physDevice, format, &formatProps);
		if (!(formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
		{
			YM_CORE_ERROR(VULKAN_PREFIX "src format does not support blitting from optimal tiled images!")
			return; // TODO
		}
		vkGetPhysicalDeviceFormatProperties(physDevice, VK_FORMAT_R8G8B8A8_SRGB, &formatProps);
		if (!(formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT))
		{
			YM_CORE_ERROR(VULKAN_PREFIX "dst format does not support blitting from optimal tiled images!")
			return; // TODO
		}

		VkImage srcImage				  = VK_NULL_HANDLE;
		AssetType textureType			  = AssetType::Texture2D;
		if (p_Texture)
		{
			textureType					  = p_Texture->GetType();
			if (textureType == AssetType::Texture2D)
			{
				srcImage				  = p_Texture.As<VulkanTexture2D>()->GetImage();
			}
			else if (textureType == AssetType::TextureArray)
			{
				srcImage				  = p_Texture.As<VulkanTextureArray>()->GetImage();
			}
		}
		else
		{
			return;
		}

		uint32_t width					  = p_Texture->GetWidth();
		uint32_t height					  = p_Texture->GetHeight();

		VkImageCreateInfo imageCreateCI   = {};
		imageCreateCI.sType				  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateCI.imageType			  = VK_IMAGE_TYPE_2D;

		imageCreateCI.format			  = VK_FORMAT_R8G8B8A8_SRGB;
		imageCreateCI.extent.width		  = width;
		imageCreateCI.extent.height		  = height;
		imageCreateCI.extent.depth		  = 1;
		imageCreateCI.arrayLayers		  = 1;
		imageCreateCI.mipLevels			  = 1;
		imageCreateCI.initialLayout		  = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateCI.samples			  = VK_SAMPLE_COUNT_1_BIT;
		imageCreateCI.tiling			  = VK_IMAGE_TILING_LINEAR;
		imageCreateCI.usage				  = VK_IMAGE_USAGE_TRANSFER_DST_BIT;


		VkImage dstImage				  = VK_NULL_HANDLE;
		auto res						  = vkCreateImage(device, &imageCreateCI, nullptr, &dstImage);
		if (res != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to create screenshot dst image!")
			return;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, dstImage, &memRequirements);

		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType				  = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize		  = memRequirements.size;
		memAllocInfo.memoryTypeIndex	  = VulkanDevice::Get().FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkDeviceMemory dstImageMemory	  = VK_NULL_HANDLE;
		res = vkAllocateMemory(device, &memAllocInfo, nullptr, &dstImageMemory);
		if (res != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to allocate screenshot dst image memory!")
			return;
		}

		res = vkBindImageMemory(device, dstImage, dstImageMemory, 0);
		if (res != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to bind screenshot dst image memory!")
			return;
		}

		auto layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		if (textureType == AssetType::Texture2D)
		{
			layout  = p_Texture.As<VulkanTexture2D>()->GetLayout();
			p_Texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}
		else if (textureType == AssetType::TextureArray)
		{
			layout  = p_Texture.As<VulkanTextureArray>()->GetLayout();
			p_Texture.As<VulkanTextureArray>()->TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		}

		VkCommandBuffer copyCmd = VKUtils::BeginSingleTimeCommand();

		VKUtils::InsertImageBarrier(
			copyCmd,
			dstImage,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

		{
			VkOffset3D blitSize						  = {};
			blitSize.x								  = width;
			blitSize.y								  = height;
			blitSize.z								  = 1;

			VkImageBlit imageBlitRegion				  = {};
			imageBlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.srcSubresource.layerCount = 1;
			imageBlitRegion.srcOffsets[1]			  = blitSize;
			imageBlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlitRegion.dstSubresource.layerCount = 1;
			imageBlitRegion.dstOffsets[1]			  = blitSize;

			vkCmdBlitImage(
				copyCmd,
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlitRegion,
				VK_FILTER_NEAREST);
		}

		VKUtils::InsertImageBarrier(
			copyCmd,
			dstImage,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

		VKUtils::EndSingleTimeCommand(copyCmd);

		if (textureType == AssetType::Texture2D)
		{
			p_Texture.As<VulkanTexture2D>()->TransitionImage(layout);
		}
		else if (textureType == AssetType::TextureArray)
		{
			p_Texture.As<VulkanTextureArray>()->TransitionImage(layout);
		}

		VkImageSubresource subResource{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout subResourceLayout;
		vkGetImageSubresourceLayout(device, dstImage, &subResource, &subResourceLayout);

		if (p_Async)
		{
			void* data = nullptr;
			vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, &data);
			data = static_cast<char*>(data) + subResourceLayout.offset;

			std::vector<uint8_t> tempData(height * subResourceLayout.rowPitch);
			memcpy(tempData.data(), data, height * subResourceLayout.rowPitch);
			vkUnmapMemory(device, dstImageMemory);

			vkDestroyImage(device, dstImage, nullptr);
			vkFreeMemory(device, dstImageMemory, nullptr);

			std::thread([tempData = std::move(tempData), p_OutPath, width, height, rowPitch = subResourceLayout.rowPitch]() mutable
			{
				std::filesystem::create_directories(std::filesystem::path(p_OutPath).parent_path());

				int32_t resWrite = stbi_write_png(
					p_OutPath.c_str(),
					width,
					height,
					4,
					tempData.data(),
					static_cast<int>(rowPitch));

				if (!resWrite)
				{
					YM_CORE_ERROR("Failed to save screenshot to disk in thread!");
				}
				else
				{
					YM_CORE_INFO("Screenshot saved asynchronously to path: {}", p_OutPath);
				}

			}).detach();
		}
		else
		{
			const char* data;
			vkMapMemory(device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&data);
			data += subResourceLayout.offset;

			std::filesystem::create_directories(std::filesystem::path(p_OutPath).parent_path());

			int32_t resWrite = stbi_write_png(
				p_OutPath.c_str(),
				width,
				height,
				4,
				data,
				(int)subResourceLayout.rowPitch);

			if (!resWrite)
			{
				YM_CORE_ERROR("Failed to save screenshot to disk!")
			}
			else
			{
				YM_CORE_INFO("Screenshot saved to path: {}", p_OutPath);
			}

			vkUnmapMemory(device, dstImageMemory);

			VKUtils::WaitIdle();
			vkFreeMemory(device, dstImageMemory, nullptr);
			vkDestroyImage(device, dstImage, nullptr);
		}
	}

	void VulkanRendererAPI::Draw(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, uint32_t p_VertexCount, uint32_t p_InstanceCount)
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle();

		if (p_VertexBuffer != nullptr)
			p_VertexBuffer->Bind(p_CommandBuffer);

		vkCmdDraw(commandBuffer, p_VertexCount, p_InstanceCount, 0, 0);
	}

	void VulkanRendererAPI::DrawIndexed(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, const Ref<IndexBuffer>& p_IndexBuffer, uint32_t p_InstanceCount)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(p_IndexBuffer)

		auto& commandBuffer = static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle();

		if (p_VertexBuffer != nullptr)
			p_VertexBuffer->Bind(p_CommandBuffer);
		
		p_IndexBuffer->Bind(p_CommandBuffer);

		vkCmdDrawIndexed(commandBuffer, p_IndexBuffer->GetCount(), p_InstanceCount, 0, 0, 0);
	}

}
