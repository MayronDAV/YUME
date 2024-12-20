#include "YUME/yumepch.h"
#include "vulkan_texture.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Core/vulkan_memory_buffer.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "vulkan_context.h"
#include "YUME/Renderer/renderer_command.h"
#include "YUME/Core/application.h"
#include "YUME/Utils/utils.h"




namespace YUME
{
	static bool HasDepthComponent(VkFormat p_Format)
	{
		return  p_Format == VK_FORMAT_D16_UNORM ||
			p_Format == VK_FORMAT_D32_SFLOAT ||
			p_Format == VK_FORMAT_D16_UNORM_S8_UINT ||
			p_Format == VK_FORMAT_D24_UNORM_S8_UINT ||
			p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	static void GenerateMipmaps(VkImage p_Image, VkFormat p_Format, int32_t p_Width, int32_t p_Height, uint32_t p_MipLevels, uint32_t p_Layer = 0, uint32_t p_LayerCount = 1)
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VulkanDevice::Get().GetPhysicalDevice(), p_Format, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		{
			YM_CORE_ERROR(VULKAN_PREFIX "texture image format does not support linear blitting!")
			return;
		}

		VkCommandBuffer commandBuffer			  = VKUtils::BeginSingleTimeCommand();

		VkImageMemoryBarrier barrier{};
		barrier.sType							  = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image							  = p_Image;
		barrier.srcQueueFamilyIndex				  = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex				  = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask		  = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer	  = p_Layer;
		barrier.subresourceRange.layerCount		  = p_LayerCount;
		barrier.subresourceRange.levelCount		  = 1;

		int32_t mipWidth = p_Width;
		int32_t mipHeight = p_Height;

		for (uint32_t i = 1; i < p_MipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout					  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout					  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask				  = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask				  = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0]					  = { 0, 0, 0 };
			blit.srcOffsets[1]					  = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask		  = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel		  = i - 1;
			blit.srcSubresource.baseArrayLayer	  = p_Layer;
			blit.srcSubresource.layerCount		  = p_LayerCount;

			blit.dstOffsets[0]					  = { 0, 0, 0 };
			blit.dstOffsets[1]					  = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask		  = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel		  = i;
			blit.dstSubresource.baseArrayLayer	  = p_Layer;
			blit.dstSubresource.layerCount		  = p_LayerCount;

			vkCmdBlitImage(commandBuffer,
				p_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				p_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout					  = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout					  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask				  = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask				  = VK_ACCESS_TRANSFER_WRITE_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}


		barrier.subresourceRange.baseMipLevel	  = p_MipLevels - 1;
		barrier.oldLayout						  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout						  = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask					  = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask					  = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VKUtils::EndSingleTimeCommand(commandBuffer);
	}

#ifndef USE_VMA_ALLOCATOR
	static void AllocateImageMemory(VkImage& p_Image, VkDeviceMemory& p_ImageMemory, VkMemoryPropertyFlags p_Properties)
	{
		YM_PROFILE_FUNCTION()

			auto device = VulkanDevice::Get().GetDevice();

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, p_Image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(memRequirements.memoryTypeBits, p_Properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &p_ImageMemory) != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to allocate texture image memory!")
				return;
		}

		vkBindImageMemory(device, p_Image, p_ImageMemory, 0);
	}
#endif

#ifdef USE_VMA_ALLOCATOR
	static void CreateImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling,
		VkImageUsageFlags p_Usage, VkImage& p_Image, VkImageCreateFlags p_Flags, uint32_t p_MipLevels, uint32_t p_LayerCount,
		VmaAllocation& p_Allocation)
#else
	static void CreateImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling,
		VkImageUsageFlags p_Usage, VkImage& p_Image, VkImageCreateFlags p_Flags, uint32_t p_MipLevels, uint32_t p_LayerCount,
		VkMemoryPropertyFlags p_Properties, VkDeviceMemory& p_ImageMemory)
#endif
	{
		YM_PROFILE_FUNCTION()

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = p_Width;
		imageInfo.extent.height = p_Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = p_MipLevels;
		imageInfo.arrayLayers = p_LayerCount;
		imageInfo.format = p_Format;
		imageInfo.tiling = p_Tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = p_Usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = p_Flags;

#ifdef USE_VMA_ALLOCATOR
		VmaAllocationCreateInfo allocInfovma = {};
		allocInfovma.flags = 0;
		allocInfovma.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		allocInfovma.requiredFlags = 0;
		allocInfovma.preferredFlags = 0;
		allocInfovma.memoryTypeBits = 0;
		allocInfovma.pool = nullptr;
		allocInfovma.pUserData = nullptr;

#ifdef USE_SMALL_VMA_POOL
		uint32_t bytesPerPixel = 8;
		uint32_t imageSize = imageInfo.extent.width * imageInfo.extent.height * imageInfo.extent.depth * bytesPerPixel;

		// If mipmaps are used, calculate the size for each level and sum them up
		for (uint32_t mipLevel = 1; mipLevel < imageInfo.mipLevels; ++mipLevel)
		{
			imageSize += std::max(1u, imageInfo.extent.width >> mipLevel) * std::max(1u, imageInfo.extent.height >> mipLevel) * imageInfo.extent.depth * bytesPerPixel;
		}

		// If there are multiple array layers, multiply by the layer count
		imageSize *= imageInfo.arrayLayers;

		if (imageSize <= SMALL_ALLOCATION_MAX_SIZE)
		{
			uint32_t mem_type_index = 0;
			vmaFindMemoryTypeIndexForImageInfo(VulkanDevice::Get().GetAllocator(), &imageInfo, &allocInfovma, &mem_type_index);
			allocInfovma.pool = VulkanDevice::Get().GetOrCreateSmallAllocPool(mem_type_index);
		}
#endif

		VmaAllocationInfo alloc_info = {};
		if (vmaCreateImage(VulkanDevice::Get().GetAllocator(), &imageInfo, &allocInfovma, &p_Image, &p_Allocation, &alloc_info) != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to create texture image!")
				return;
		}
#else
		if (vkCreateImage(VulkanDevice::Get().GetDevice(), &imageInfo, VK_NULL_HANDLE, &p_Image) != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to create texture image!")
				return;
		}


		AllocateImageMemory(p_Image, p_ImageMemory, p_Properties);
#endif
	}


	static VkImageView CreateImageView(VkImage p_Image, VkFormat p_Format, VkImageViewType p_ViewType, uint32_t p_MipLevels, uint32_t p_LayerCount)
	{
		YM_PROFILE_FUNCTION()

		auto device = VulkanDevice::Get().GetDevice();

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType		= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image		= p_Image;
		viewInfo.viewType	= p_ViewType;
		viewInfo.format		= p_Format;

		if (p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || p_Format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else if (HasDepthComponent(p_Format))
		{
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = p_MipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = p_LayerCount;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to create texture image view!")
				return nullptr;
		}

		return imageView;
	}

	static VkSampler CreateImageSampler(TextureFilter p_MinFilter, TextureFilter p_MagFilter, TextureWrap  p_WrapU, TextureWrap p_WrapV, TextureWrap p_WrapW,
		VkFormat p_Format, bool p_AnisotropyEnable, float p_MaxLod, TextureBorderColor p_BorderColorFlag, glm::vec4 p_BorderColor)
	{
		VkSamplerCreateInfo samplerInfo		= {};
		samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext					= VK_NULL_HANDLE;
		samplerInfo.minFilter				= VKUtils::TextureFilterToVk(p_MinFilter);
		samplerInfo.magFilter				= VKUtils::TextureFilterToVk(p_MagFilter);
		samplerInfo.addressModeU			= VKUtils::TextureWrapToVk(p_WrapU);
		samplerInfo.addressModeV			= VKUtils::TextureWrapToVk(p_WrapV);
		samplerInfo.addressModeW			= VKUtils::TextureWrapToVk(p_WrapW);

		if (p_AnisotropyEnable && RendererCommand::GetCapabilities().SamplerAnisotropy)
		{
			samplerInfo.anisotropyEnable	= VK_TRUE;
			auto properties					= VulkanDevice::Get().GetPhysicalDeviceStruct().Properties;
			samplerInfo.maxAnisotropy		= properties.limits.maxSamplerAnisotropy;
		}
		else
		{
			samplerInfo.anisotropyEnable	= VK_FALSE;
			samplerInfo.maxAnisotropy		= 1.0f;
		}

		samplerInfo.maxLod					= p_MaxLod;

		auto borderColor					= VKUtils::TextureBorderColorToVk(p_BorderColorFlag);
		VkSamplerCustomBorderColorCreateInfoEXT borderColorCI{};
		{
			samplerInfo.compareEnable		= VK_FALSE;
			samplerInfo.compareOp			= VK_COMPARE_OP_ALWAYS;
			samplerInfo.borderColor			= borderColor;

			if (borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT)
			{
				borderColorCI.sType						 = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
				borderColorCI.customBorderColor.int32[0] = (int32_t)p_BorderColor.x;
				borderColorCI.customBorderColor.int32[1] = (int32_t)p_BorderColor.y;
				borderColorCI.customBorderColor.int32[2] = (int32_t)p_BorderColor.z;
				borderColorCI.customBorderColor.int32[3] = (int32_t)p_BorderColor.w;
				borderColorCI.format					 = p_Format;
				samplerInfo.pNext						 = &borderColorCI;
			}
			else if (borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT)
			{
				borderColorCI.sType						   = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
				borderColorCI.customBorderColor.float32[0] = p_BorderColor.x;
				borderColorCI.customBorderColor.float32[1] = p_BorderColor.y;
				borderColorCI.customBorderColor.float32[2] = p_BorderColor.z;
				borderColorCI.customBorderColor.float32[3] = p_BorderColor.w;
				borderColorCI.format					   = p_Format;
				samplerInfo.pNext						   = &borderColorCI;
			}
		}

		samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.mipLodBias				= 0.0f;
		samplerInfo.minLod					= 0.0f;

		VkSampler sampler;
		if (vkCreateSampler(VulkanDevice::Get().GetDevice(), &samplerInfo, VK_NULL_HANDLE, &sampler) != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to create texture sampler!")
			return VK_NULL_HANDLE;
		}

		return sampler;
	}

	static VkImageSubresourceRange GetRange(TextureFormat p_Format)
	{
		VkImageSubresourceRange range{};

		if (Texture::IsDepthFormat(p_Format) && Texture::IsStencilFormat(p_Format))
		{
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else if (Texture::IsDepthFormat(p_Format))
		{
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		range.baseMipLevel   = 0;
		range.baseArrayLayer = 0;

		return range;
	}

	#pragma region TEXTURE_2D

	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		Init(p_Spec);

		switch (p_Spec.Usage)
		{
			case TextureUsage::TEXTURE_COLOR_ATTACHMENT:
				TransitionImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); break;
			case TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT:
				TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); break;
		}
	}

	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& p_Spec, const unsigned char* p_Data, size_t p_Size)
	{
		YM_PROFILE_FUNCTION()

		Init(p_Spec);

		auto stagingBuffer = new VulkanMemoryBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, p_Size);

		stagingBuffer->SetData(p_Size, p_Data);

		TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VKUtils::CopyBufferToImage(stagingBuffer->GetBuffer(), m_TextureImage, p_Spec.Width, p_Spec.Height, m_VkFormat);

		stagingBuffer->SetDeleteWithoutQueue(true);
		delete stagingBuffer;

		if (p_Spec.GenerateMips && p_Spec.Width > 1 && p_Spec.Height > 1 && m_MipLevels > 1)
		{
			GenerateMipmaps(m_TextureImage, m_VkFormat, p_Spec.Width, p_Spec.Height, m_MipLevels);
		}

		TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	VulkanTexture2D::VulkanTexture2D(VkImage p_Image, VkImageView p_ImageView, VkFormat p_Format, uint32_t p_Width, uint32_t p_Height)
	{
		YM_PROFILE_FUNCTION()

		m_TextureImage = p_Image;
		m_TextureImageView = p_ImageView;
		m_VkFormat = p_Format;
		m_Specification.Width = p_Width;
		m_Specification.Height = p_Height;

		m_ShouldDestroy = false;
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		YM_PROFILE_FUNCTION()

		if (!m_ShouldDestroy)
			return;

		auto image = m_TextureImage;
		auto imageView = m_TextureImageView;
		auto sampler = m_TextureSampler;

		VulkanContext::PushFunction([imageView, sampler]()
		{
			auto device = VulkanDevice::Get().GetDevice();

			if (sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(device, sampler, VK_NULL_HANDLE);
			}

			if (imageView != VK_NULL_HANDLE)
			{
				vkDestroyImageView(device, imageView, VK_NULL_HANDLE);
			}
		});

#ifdef USE_VMA_ALLOCATOR
		auto alloc = m_Allocation;
		VulkanContext::PushFunction([image, alloc]()
#else
		auto memory = m_TextureImageMemory;
		VulkanContext::PushFunction([image, memory]()
#endif
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Destroying texture 2D image...")

#ifdef USE_VMA_ALLOCATOR		
			vmaDestroyImage(VulkanDevice::Get().GetAllocator(), image, alloc);
#else
			auto device = VulkanDevice::Get().GetDevice();

			vkDestroyImage(device, image, VK_NULL_HANDLE);
			vkFreeMemory(device, memory, VK_NULL_HANDLE);
#endif
		});

		m_TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	void VulkanTexture2D::Resize(uint32_t p_Width, uint32_t p_Height)
	{
		if (p_Width == 0 || p_Height == 0) return;
		if (m_Specification.Width == p_Width && m_Specification.Height == p_Height) return;

		auto image = m_TextureImage;
		auto imageView = m_TextureImageView;
		auto sampler = m_TextureSampler;
#ifdef USE_VMA_ALLOCATOR
		auto alloc = m_Allocation;
		VulkanContext::PushFunction([image, imageView, sampler, alloc]()
#else
		auto memory = m_TextureImageMemory;
		VulkanContext::PushFunction([image, imageView, sampler, memory]()
#endif
			{
				YM_CORE_TRACE(VULKAN_PREFIX "Destroying texture image...")

					auto device = VulkanDevice::Get().GetDevice();

				vkDestroySampler(device, sampler, VK_NULL_HANDLE);

				vkDestroyImageView(device, imageView, VK_NULL_HANDLE);

#ifdef USE_VMA_ALLOCATOR		
				vmaDestroyImage(VulkanDevice::Get().GetAllocator(), image, alloc);
#else
				vkDestroyImage(device, image, VK_NULL_HANDLE);
				vkFreeMemory(device, memory, VK_NULL_HANDLE);
#endif
			});

		m_Specification.Width = p_Width;
		m_Specification.Height = p_Height;

		Init(m_Specification);
	}

	void VulkanTexture2D::SetData(const void* p_Data, size_t p_Size)
	{
		auto stagingBuffer = new VulkanMemoryBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, p_Size);

		stagingBuffer->SetData(p_Size, p_Data);

		TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VKUtils::CopyBufferToImage(stagingBuffer->GetBuffer(), m_TextureImage, m_Specification.Width, m_Specification.Height, m_VkFormat);

		stagingBuffer->SetDeleteWithoutQueue(true);
		delete stagingBuffer;

		m_TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (m_Specification.Usage == TextureUsage::TEXTURE_SAMPLED)
			TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else if (m_Specification.Usage == TextureUsage::TEXTURE_STORAGE)
			TransitionImage(VK_IMAGE_LAYOUT_GENERAL);
	}


	VkImageSubresourceRange VulkanTexture2D::GetSubresourceRange() const
	{
		VkImageSubresourceRange range = GetRange(m_Specification.Format);
		range.levelCount			  = m_MipLevels;
		range.layerCount			  = 1;

		return range;
	}

	void VulkanTexture2D::TransitionImage(VkImageLayout p_NewLayout, CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		if (p_NewLayout != m_TextureImageLayout)
		{
			VKUtils::TransitionImageLayout(m_TextureImage, m_VkFormat, m_TextureImageLayout, p_NewLayout,
				p_CommandBuffer ? static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle() : VK_NULL_HANDLE, 0, m_MipLevels);
			m_TextureImageLayout = p_NewLayout;
		}
	}

	bool VulkanTexture2D::operator==(const Texture& p_Other) const
	{
		auto texture = (VulkanTexture2D*)&p_Other;

		return (
			m_Specification.DebugName == texture->m_Specification.DebugName &&
			m_TextureImage			  == texture->m_TextureImage			&&
			m_TextureImageView		  == texture->m_TextureImageView		&&
			m_TextureImageLayout	  == texture->m_TextureImageLayout		&&
			m_VkFormat				  == texture->m_VkFormat				&&
			m_MipLevels				  == texture->m_MipLevels
		);
	}

	void VulkanTexture2D::Init(const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		m_Specification		= p_Spec;
		m_Channels			= VKUtils::TextureFormatChannels(p_Spec.Format);
		m_VkFormat			= VKUtils::TextureFormatToVk(p_Spec.Format);
		m_BytesPerChannel	= VKUtils::TextureFormatBytesPerChannel(p_Spec.Format);
		auto usageFlagBits	= VKUtils::TextureUsageToVk(p_Spec.Usage);
		if (p_Spec.Usage != TextureUsage::TEXTURE_STORAGE)
		{
			usageFlagBits |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		usageFlagBits |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		usageFlagBits |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (p_Spec.GenerateMips && p_Spec.Width > 1 && p_Spec.Height > 1 && p_Spec.Usage == TextureUsage::TEXTURE_SAMPLED)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(p_Spec.Width, p_Spec.Height)))) + 1;
		}

		VkImageCreateFlags cflags = 0;

#ifdef USE_VMA_ALLOCATOR
		CreateImage(p_Spec.Width, p_Spec.Height, m_VkFormat,
			VK_IMAGE_TILING_OPTIMAL, usageFlagBits,
			m_TextureImage, cflags, m_MipLevels, 1, m_Allocation);
#else
		CreateImage(p_Spec.Width, p_Spec.Height, m_VkFormat,
			VK_IMAGE_TILING_OPTIMAL, usageFlagBits, m_TextureImage, cflags, m_MipLevels, 1,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImageMemory);
#endif

		if (!p_Spec.DebugName.empty())
		{
			std::string debugName = p_Spec.DebugName + " - VkImage";
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_IMAGE, debugName.c_str(), m_TextureImage);
		}

		m_TextureImageView = CreateImageView(m_TextureImage, m_VkFormat, VK_IMAGE_VIEW_TYPE_2D, m_MipLevels, 1);

		if (!p_Spec.DebugName.empty())
		{
			std::string debugName = p_Spec.DebugName + " - VkImageView";
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, debugName.c_str(), m_TextureImageView);
		}

		if (p_Spec.Usage != TextureUsage::TEXTURE_STORAGE)
		{
			float maxLod = 0.0f;
			if (p_Spec.GenerateMips && m_MipLevels > 1)
			{
				maxLod = float(m_MipLevels);
			}
			m_TextureSampler = CreateImageSampler(p_Spec.MinFilter, p_Spec.MagFilter, p_Spec.WrapU, p_Spec.WrapV, p_Spec.WrapW, m_VkFormat,
				p_Spec.AnisotropyEnable, maxLod, p_Spec.BorderColorFlag, p_Spec.BorderColor);

			if (!p_Spec.DebugName.empty())
			{
				std::string debugName = p_Spec.DebugName + " - VkSampler";
				VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_SAMPLER, debugName.c_str(), m_TextureSampler);
			}
		}
	}

	#pragma endregion

	#pragma region TEXTURE_ARRAY

	VulkanTextureArray::VulkanTextureArray(const TextureArraySpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		Init(p_Spec);

		switch (p_Spec.Spec.Usage)
		{
			case TextureUsage::TEXTURE_COLOR_ATTACHMENT:
				TransitionImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL); break;
			case TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT:
				TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL); break;
		}
	}

	VulkanTextureArray::VulkanTextureArray(const TextureArraySpecification& p_Spec, const uint8_t* p_Data, size_t p_Size)
	{
		Init(p_Spec);

		auto stagingBuffer = new VulkanMemoryBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, p_Size);

		stagingBuffer->SetData(p_Size, p_Data);

		TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkCommandBuffer commandBuffer						 = VKUtils::BeginSingleTimeCommand();;

		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset										 = 0;

		auto width											 = p_Spec.Spec.Width;
		auto height											 = p_Spec.Spec.Height;

		for (uint32_t layer = 0; layer < p_Spec.Count; layer++)
		{
			VkBufferImageCopy bufferCopyRegion				 = {};
			bufferCopyRegion.imageSubresource.aspectMask	 = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel		 = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = layer;
			bufferCopyRegion.imageSubresource.layerCount	 = 1;
			bufferCopyRegion.imageExtent.width				 = width;
			bufferCopyRegion.imageExtent.height				 = height;
			bufferCopyRegion.imageExtent.depth				 = 1;
			bufferCopyRegion.bufferOffset					 = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);

			offset											+= width * height * m_Channels * m_BytesPerChannel;
		}

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer->GetBuffer(),
			m_TextureImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data());

		VKUtils::EndSingleTimeCommand(commandBuffer);

		if (p_Spec.Spec.GenerateMips && width > 1 && height > 1 && m_MipLevels > 1)
		{
			for (uint32_t i = 0; i < m_LayerCount; i++)
			{
				GenerateMipmaps(m_TextureImage, m_VkFormat, width, height, m_MipLevels, i, 1);
			}
		}

		TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		stagingBuffer->SetDeleteWithoutQueue(true);
		delete stagingBuffer;
	}

	VulkanTextureArray::~VulkanTextureArray()
	{
		YM_PROFILE_FUNCTION()

		auto image		= m_TextureImage;
		auto imageView  = m_TextureImageView;
		auto sampler	= m_TextureSampler;

		VulkanContext::PushFunction([imageView, sampler]()
		{
			auto device = VulkanDevice::Get().GetDevice();

			if (sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(device, sampler, VK_NULL_HANDLE);
			}

			if (imageView != VK_NULL_HANDLE)
			{
				vkDestroyImageView(device, imageView, VK_NULL_HANDLE);
			}
		});

#ifdef USE_VMA_ALLOCATOR
		auto alloc = m_Allocation;
		VulkanContext::PushFunction([image, alloc]()
#else
		auto memory = m_TextureImageMemory;
		VulkanContext::PushFunction([image, memory]()
#endif
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Destroying texture array image...")

#ifdef USE_VMA_ALLOCATOR		
			vmaDestroyImage(VulkanDevice::Get().GetAllocator(), image, alloc);
#else
			auto device = VulkanDevice::Get().GetDevice();

			vkDestroyImage(device, image, VK_NULL_HANDLE);
			vkFreeMemory(device, memory, VK_NULL_HANDLE);
#endif
		});

		m_TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	}

	void VulkanTextureArray::TransitionImage(VkImageLayout p_NewLayout, CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		if (p_NewLayout != m_TextureImageLayout)
		{
			VKUtils::TransitionImageLayout(m_TextureImage, m_VkFormat, m_TextureImageLayout, p_NewLayout,
				(p_CommandBuffer) ? static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle() : VK_NULL_HANDLE, 0, m_MipLevels, 0, m_LayerCount);
			m_TextureImageLayout = p_NewLayout;
		}
	}

	void VulkanTextureArray::SetData(const void* p_Data, size_t p_Size)
	{
		auto stagingBuffer = new VulkanMemoryBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, p_Size);

		stagingBuffer->SetData(p_Size, p_Data);

		TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		VKUtils::CopyBufferToImage(stagingBuffer->GetBuffer(), m_TextureImage, m_Specification.Spec.Width, m_Specification.Spec.Height, m_VkFormat, m_LayerCount);

		stagingBuffer->SetDeleteWithoutQueue(true);
		delete stagingBuffer;

		if (m_Specification.Spec.Usage == TextureUsage::TEXTURE_SAMPLED)
			TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else if (m_Specification.Spec.Usage == TextureUsage::TEXTURE_STORAGE)
			TransitionImage(VK_IMAGE_LAYOUT_GENERAL);
	}

	VkImageSubresourceRange VulkanTextureArray::GetSubresourceRange() const
	{
		VkImageSubresourceRange range = GetRange(m_Specification.Spec.Format);
		range.levelCount			  = m_MipLevels;
		range.layerCount			  = m_LayerCount;

		return range;
	}

	bool VulkanTextureArray::operator==(const Texture& p_Other) const
	{
		auto texture = (VulkanTextureArray*)&p_Other;

		return (
			m_Specification.Spec.DebugName  == texture->m_Specification.Spec.DebugName  &&
			m_TextureImage					== texture->m_TextureImage					&&
			m_TextureImageView				== texture->m_TextureImageView				&&
			m_TextureImageLayout			== texture->m_TextureImageLayout			&&
			m_VkFormat						== texture->m_VkFormat						&&
			m_MipLevels						== texture->m_MipLevels						&&
			m_LayerCount					== texture->m_LayerCount
		);
	}

	void VulkanTextureArray::Init(const TextureArraySpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		m_Specification		= p_Spec;
		auto& spec			= p_Spec.Spec;
		m_LayerCount		= p_Spec.Count;
		m_Channels			= VKUtils::TextureFormatChannels(spec.Format);
		m_VkFormat			= VKUtils::TextureFormatToVk(spec.Format);
		m_BytesPerChannel	= VKUtils::TextureFormatBytesPerChannel(spec.Format);
		auto usageFlagBits  = VKUtils::TextureUsageToVk(spec.Usage);
		if (spec.Usage != TextureUsage::TEXTURE_STORAGE)
		{
			usageFlagBits  |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		usageFlagBits	   |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		usageFlagBits	   |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (spec.GenerateMips && spec.Width > 1 && spec.Height > 1)
		{
			m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(spec.Width, spec.Height)))) + 1;
		}
		
		VkImageCreateFlags cFlags = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;

		if (p_Spec.Type == TextureArrayType::CubeMap)
		{
			cFlags				  = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

#ifdef USE_VMA_ALLOCATOR
		CreateImage(spec.Width, spec.Height, m_VkFormat,
			VK_IMAGE_TILING_OPTIMAL, usageFlagBits,
			m_TextureImage, cFlags, m_MipLevels, m_LayerCount,
			m_Allocation);
#else
		CreateImage(spec.Width, spec.Height, m_VkFormat,
			VK_IMAGE_TILING_OPTIMAL, usageFlagBits,
			m_TextureImage, cFlags, m_MipLevels, m_LayerCount,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImageMemory);
#endif

		if (!spec.DebugName.empty())
		{
			std::string debugName = spec.DebugName + " - VkImage";
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_IMAGE, debugName.c_str(), m_TextureImage);
		}

		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

		if (p_Spec.Type == TextureArrayType::CubeMap)
		{
			viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}

		m_TextureImageView = CreateImageView(m_TextureImage, m_VkFormat, viewType, m_MipLevels, m_LayerCount);

		if (!spec.DebugName.empty())
		{
			std::string debugName = spec.DebugName + " - VkImageView";
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, debugName.c_str(), m_TextureImageView);
		}

		if (spec.Usage != TextureUsage::TEXTURE_STORAGE)
		{
			float maxLod = 0.0f;
			if (spec.GenerateMips && m_MipLevels > 1)
			{
				maxLod = float(m_MipLevels);
			}
			m_TextureSampler = CreateImageSampler(spec.MinFilter, spec.MagFilter, spec.WrapU, spec.WrapV, spec.WrapW, m_VkFormat,
				spec.AnisotropyEnable, maxLod, spec.BorderColorFlag, spec.BorderColor);

			if (!spec.DebugName.empty())
			{
				std::string debugName = spec.DebugName + " - VkSampler";
				VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_SAMPLER, debugName.c_str(), m_TextureSampler);
			}
		}
	}

	#pragma endregion
}
