#include "YUME/yumepch.h"
#include "vulkan_texture.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Core/vulkan_memory_buffer.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "vulkan_context.h"




namespace YUME
{
	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& p_Spec)
	{
		Init(p_Spec);
	}

	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size)
	{
		YM_PROFILE_FUNCTION()

		Init(p_Spec);

		auto stagingBuffer = new VulkanMemoryBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, p_Size);

		stagingBuffer->SetData(p_Size, p_Data);

		TransitionImage(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		Utils::CopyBufferToImage(stagingBuffer->GetBuffer(), m_TextureImage, p_Spec.Width, p_Spec.Height);

		stagingBuffer->SetDeleteWithoutQueue(true);
		delete stagingBuffer;

		m_TextureImageView = CreateImageView(m_TextureImage, m_VkFormat);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = VK_NULL_HANDLE;
		samplerInfo.magFilter = Utils::TextureFilterToVk(p_Spec.MagFilter);
		samplerInfo.minFilter = Utils::TextureFilterToVk(p_Spec.MinFilter);
		samplerInfo.addressModeU = Utils::TextureWrapToVk(p_Spec.WrapU);
		samplerInfo.addressModeV = Utils::TextureWrapToVk(p_Spec.WrapV);
		samplerInfo.addressModeW = Utils::TextureWrapToVk(p_Spec.WrapW);
		samplerInfo.anisotropyEnable = VK_TRUE;
		auto properties = VulkanDevice::Get().GetPhysicalDeviceStruct().Properties;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		auto borderColor = Utils::TextureBorderColorToVk(p_Spec.BorderColorFlag);
		samplerInfo.borderColor = borderColor;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSamplerCustomBorderColorCreateInfoEXT borderColorCI{};
		if (borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT)
		{		
			borderColorCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
			borderColorCI.customBorderColor.int32[0] = (int32_t)p_Spec.BorderColor.x;
			borderColorCI.customBorderColor.int32[1] = (int32_t)p_Spec.BorderColor.y;
			borderColorCI.customBorderColor.int32[2] = (int32_t)p_Spec.BorderColor.z;
			borderColorCI.customBorderColor.int32[3] = (int32_t)p_Spec.BorderColor.w;
			borderColorCI.format = m_VkFormat;
			samplerInfo.pNext = &borderColorCI;
		}
		else if (borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT)
		{
			borderColorCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
			borderColorCI.customBorderColor.float32[0] = p_Spec.BorderColor.x;
			borderColorCI.customBorderColor.float32[1] = p_Spec.BorderColor.y;
			borderColorCI.customBorderColor.float32[2] = p_Spec.BorderColor.z;
			borderColorCI.customBorderColor.float32[3] = p_Spec.BorderColor.w;
			borderColorCI.format = m_VkFormat;
			samplerInfo.pNext = &borderColorCI;
		}

		if (vkCreateSampler(VulkanDevice::Get().GetDevice(), &samplerInfo, VK_NULL_HANDLE, &m_TextureSampler) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to create texture sampler!")
			return;
		}

		TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}



	VulkanTexture2D::VulkanTexture2D(VkImage p_Image, VkImageView p_ImageView, VkFormat p_Format, uint32_t p_Width, uint32_t p_Height)
	{
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		YM_PROFILE_FUNCTION()

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
			YM_CORE_TRACE("Destroying vulkan texture image...")

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
	}

	void VulkanTexture2D::SetData(const unsigned char* p_Data, uint32_t p_Size)
	{
	}

	void VulkanTexture2D::Bind(uint32_t p_Slot) const
	{
	}

	void VulkanTexture2D::Unbind(uint32_t p_Slot) const
	{
	}

	void VulkanTexture2D::TransitionImage(VkImageLayout p_NewLayout)
	{
		YM_PROFILE_FUNCTION()

		if (p_NewLayout != m_TextureImageLayout)
		{
			Utils::TransitionImageLayout(m_TextureImage, m_TextureImageLayout, p_NewLayout);
			m_TextureImageLayout = p_NewLayout;
		}
	}

	bool VulkanTexture2D::operator==(const Texture& p_Other) const
	{
		auto texture = (VulkanTexture2D*)&p_Other;

		return (
			m_TextureImage == texture->m_TextureImage &&
			m_TextureImageView == texture->m_TextureImageView &&
			m_TextureImageLayout == texture->m_TextureImageLayout &&
			m_VkFormat == texture->m_VkFormat
		);
	}

	void VulkanTexture2D::Init(const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		m_Specification = p_Spec;
		m_Channels = Utils::TextureFormatChannels(p_Spec.Format);
		m_VkFormat = Utils::TextureFormatToVk(p_Spec.Format);
		m_BytesPerChannel = Utils::TextureFormatBytesPerChannel(p_Spec.Format);

#ifdef USE_VMA_ALLOCATOR
		CreateImage(p_Spec.Width, p_Spec.Height, m_VkFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | Utils::TextureUsageToVk(p_Spec.Usage),
			m_TextureImage, m_Allocation);
#else
		CreateImage(p_Spec.Width, p_Spec.Height, m_VkFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | Utils::TextureUsageToVk(p_Spec.Usage),
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);
#endif
	}

#ifdef USE_VMA_ALLOCATOR
	void VulkanTexture2D::CreateImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling,
		VkImageUsageFlags p_Usage, VkImage& p_Image, VmaAllocation& p_Allocation)
#else
	void VulkanTexture2D::CreateImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling,
		VkImageUsageFlags p_Usage, VkMemoryPropertyFlags p_Properties, VkImage& p_Image, VkDeviceMemory& p_ImageMemory)
#endif
	{
		YM_PROFILE_FUNCTION()

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = p_Width;
		imageInfo.extent.height = p_Height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = p_Format;
		imageInfo.tiling = p_Tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = p_Usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

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
			YM_CORE_ERROR("Failed to create texture image!")
			return;
		}
#else
		if (vkCreateImage(VulkanDevice::Get().GetDevice(), &imageInfo, VK_NULL_HANDLE, &p_Image) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to create texture image!")
			return;
		}


		AllocateImageMemory(p_Image, p_ImageMemory, p_Properties);
#endif
	}

#ifndef USE_VMA_ALLOCATOR
	void VulkanTexture2D::AllocateImageMemory(VkImage& p_Image, VkDeviceMemory& p_ImageMemory, VkMemoryPropertyFlags p_Properties)
	{
		YM_PROFILE_FUNCTION()

		auto device = VulkanDevice::Get().GetDevice();
		
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, p_Image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(memRequirements.memoryTypeBits, p_Properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &p_ImageMemory) != VK_SUCCESS) {
			YM_CORE_ERROR("Failed to allocate texture image memory!")
				return;
		}

		vkBindImageMemory(device, p_Image, p_ImageMemory, 0);
	}
#endif

	VkImageView VulkanTexture2D::CreateImageView(VkImage p_Image, VkFormat p_Format)
	{
		YM_PROFILE_FUNCTION()

		auto device = VulkanDevice::Get().GetDevice();

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = p_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = p_Format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			YM_CORE_ERROR("Failed to create texture image view!")
			return nullptr;
		}

		return imageView;
	}
}
