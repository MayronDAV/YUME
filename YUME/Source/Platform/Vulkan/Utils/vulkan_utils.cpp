#include "YUME/yumepch.h"
#include "vulkan_utils.h"

#include "Platform/Vulkan/Core/vulkan_device.h"


namespace YUME::Utils
{
	std::string GetMessageSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity)
	{
		switch (p_Severity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:		 return "Verbose";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:			 return "Info";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:		 return "Warning";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:			 return "Error";
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: return "Critical";
			default:
				YM_CORE_ERROR("Unknown Message Severity!")
				return "Trace";
		}
	}

	std::string GetMessageType(VkDebugUtilsMessageTypeFlagsEXT p_Type)
	{
		switch (p_Type)
		{
			case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:				 return "General";
			case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:			 return "Validation";
			case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:			 return "Performance";

	#ifdef _WIN64 // doesn't work on my Linux for some reason
			case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT: return "Device address binding";
	#endif
			default:
				YM_CORE_ERROR("Unknown Message Type!")
				return "General";
		}
	}

	VkPolygonMode PolygonModeToVk(PolygonMode p_Mode)
	{
		switch (p_Mode)
		{
			case YUME::PolygonMode::FILL:  return VK_POLYGON_MODE_FILL;
			case YUME::PolygonMode::LINE:  return VK_POLYGON_MODE_LINE;
			case YUME::PolygonMode::POINT: return VK_POLYGON_MODE_POINT;
			default:
				YM_CORE_ERROR("Unknown Polygon Mode!")
				return VK_POLYGON_MODE_FILL;
		}
	}

	VkPrimitiveTopology DrawTypeToVk(DrawType p_Type)
	{
		switch (p_Type)
		{
			case YUME::DrawType::TRIANGLE: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			case YUME::DrawType::LINES:    return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			case YUME::DrawType::POINT:    return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			default:
				YM_CORE_ERROR("Unknown Draw Type!")
				return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	VkFrontFace FrontFaceToVk(FrontFace p_Face)
	{
		switch (p_Face)
		{
			case YUME::FrontFace::CLOCKWISE:		 return VK_FRONT_FACE_CLOCKWISE;
			case YUME::FrontFace::COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
			default:
				YM_CORE_ERROR("Unknown Front Face!")
				return VK_FRONT_FACE_CLOCKWISE;
		}
	}

	VkCullModeFlags CullModeToVk(CullMode p_Mode)
	{
		switch (p_Mode)
		{
			case CullMode::BACK:		 return VK_CULL_MODE_BACK_BIT;
			case CullMode::FRONT:		 return VK_CULL_MODE_FRONT_BIT;
			case CullMode::FRONTANDBACK: return VK_CULL_MODE_FRONT_AND_BACK;
			case CullMode::NONE:		 return VK_CULL_MODE_NONE;
			default:
				YM_CORE_ERROR("Unknown Cull Mode!")
				return VK_CULL_MODE_BACK_BIT;
		}
	}

	static bool HasDepthComponent(VkFormat p_Format) 
	{
		return  p_Format == VK_FORMAT_D16_UNORM ||
				p_Format == VK_FORMAT_D32_SFLOAT ||
				p_Format == VK_FORMAT_D16_UNORM_S8_UINT ||
				p_Format == VK_FORMAT_D24_UNORM_S8_UINT ||
				p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
	}


	void TransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_CurrentLayout, VkImageLayout p_NewLayout)
	{
		YM_PROFILE_FUNCTION()

		auto commandBuffer = BeginSingleTimeCommand();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = p_CurrentLayout;
		barrier.newLayout = p_NewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = p_Image;
		
		if (p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT || p_Format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else if (HasDepthComponent(p_Format))
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if (p_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = 0;

			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else 
		{
			YM_CORE_ASSERT(false, "Unsupported layout transition!")
			EndSingleTimeCommand(commandBuffer);
			return;
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommand(commandBuffer);
	}


	VkRenderingAttachmentInfo AttachmentInfo(VkImageView p_View, VkClearValue* p_Clear, VkImageLayout p_Layout)
	{
		VkRenderingAttachmentInfo colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.pNext = nullptr;

		colorAttachment.imageView = p_View;
		colorAttachment.imageLayout = p_Layout;
		colorAttachment.loadOp = p_Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		if (p_Clear)
			colorAttachment.clearValue = *p_Clear;

		return colorAttachment;
	}

	VkCommandBuffer BeginSingleTimeCommand()
	{
		YM_PROFILE_FUNCTION()

		auto commandPool = VulkanDevice::Get().GetCommandPool();
		auto& device = VulkanDevice::Get().GetDevice();

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommand(VkCommandBuffer p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		auto commandPool = VulkanDevice::Get().GetCommandPool();
		auto& device = VulkanDevice::Get().GetDevice();

		vkEndCommandBuffer(p_CommandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &p_CommandBuffer;

		auto& graphicQueue = VulkanDevice::Get().GetGraphicQueue();

		vkQueueSubmit(graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &p_CommandBuffer);
	}

	VkFormat TextureFormatToVk(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			using enum YUME::TextureFormat;

			case None:				 return VK_FORMAT_UNDEFINED;
			case R8_SRGB:			 return VK_FORMAT_R8_SRGB;
			case R8_INT:			 return VK_FORMAT_R8_SINT;
			case R8_UINT:			 return VK_FORMAT_R8_UINT;
			case R32_INT:			 return VK_FORMAT_R32_SINT;

			case RG8_SRGB:			 return VK_FORMAT_R8G8_SRGB;
			case RG32_UINT:			 return VK_FORMAT_R32G32_UINT;

			case RGB8_SRGB:			 return VK_FORMAT_R8G8B8_SRGB;

			case RGBA8_SRGB:		 return VK_FORMAT_R8G8B8A8_SRGB;
			case RGBA32_FLOAT:		 return VK_FORMAT_R32G32B32A32_SFLOAT;

			case D16_UNORM:			 return VK_FORMAT_D16_UNORM;
			case D32_FLOAT:			 return VK_FORMAT_D32_SFLOAT;
			case D16_UNORM_S8_UINT:  return VK_FORMAT_D16_UNORM_S8_UINT;
			case D24_UNORM_S8_UINT:  return VK_FORMAT_D24_UNORM_S8_UINT;
			case D32_FLOAT_S8_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;

			default:
				YM_CORE_ERROR("Unknown texture format!")
				return VK_FORMAT_UNDEFINED;
		}
	}

	uint32_t TextureFormatChannels(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			using enum YUME::TextureFormat;

			case R8_SRGB:
			case R8_INT:
			case R8_UINT:
			case R32_INT:
				return 1;

			case RG8_SRGB:
			case RG32_UINT:
				return 2;

			case RGB8_SRGB:
				return 3;

			case RGBA8_SRGB:
			case RGBA32_FLOAT:
				return 4;

			case D16_UNORM:
			case D32_FLOAT:
				return 1;

			case D16_UNORM_S8_UINT:
			case D24_UNORM_S8_UINT:
			case D32_FLOAT_S8_UINT:
				return 2;

			default:
				YM_CORE_ERROR("Unknown texture format!")
				return 0;
		}
	}

	uint32_t TextureFormatBytesPerChannel(TextureFormat p_Format)
	{
		switch (p_Format)
		{
			using enum YUME::TextureFormat;

			case R8_SRGB:
			case R8_INT:
			case RG8_SRGB:
			case R8_UINT:
			case RGB8_SRGB:
			case RGBA8_SRGB:
				return 1;

			case R32_INT:
			case RG32_UINT:
			case RGBA32_FLOAT:
			case D32_FLOAT:
			case D24_UNORM_S8_UINT:
				return 4;

			case D16_UNORM:			 return 2;
			case D16_UNORM_S8_UINT:  return 3;
			case D32_FLOAT_S8_UINT:  return 5;

			default:
				YM_CORE_ERROR("Unknown texture format!")
				return 1;
		}
	}

	VkFilter TextureFilterToVk(TextureFilter p_Filter)
	{
		switch (p_Filter)
		{
			case YUME::TextureFilter::LINEAR:  return VK_FILTER_LINEAR;
			case YUME::TextureFilter::NEAREST: return VK_FILTER_NEAREST;
			default: 
				YM_CORE_WARN("Unknown texture filter, returning default...")
				return VK_FILTER_LINEAR;
		}
	}

	VkSamplerAddressMode TextureWrapToVk(TextureWrap p_Wrap)
	{
		switch (p_Wrap)
		{
			case YUME::TextureWrap::REPEAT:			 return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case YUME::TextureWrap::MIRRORED_REPEAT: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case YUME::TextureWrap::CLAMP_TO_EDGE:   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case YUME::TextureWrap::CLAMP_TO_BORDER: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			default:
				YM_CORE_WARN("Unknown texture wrap, returning default...")
				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}

	VkBorderColor TextureBorderColorToVk(TextureBorderColor p_BorderColor)
	{
		switch (p_BorderColor)
		{
			using enum YUME::TextureBorderColor;

			case TRANSPARENT_BLACK_FLOAT:  return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case TRANSPARENT_BLACK_SRGB:   return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
			case OPAQUE_BLACK_FLOAT:	   return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case OPAQUE_BLACK_SRGB:		   return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case OPAQUE_WHITE_FLOAT:	   return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case OPAQUE_WHITE_SRGB:		   return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			case CUSTOM_FLOAT:			   return VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
			case CUSTOM_SRGB:			   return VK_BORDER_COLOR_INT_CUSTOM_EXT;
			default:
				YM_CORE_WARN("Unknown texture border color, returning default...")
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		}
	}

	VkImageUsageFlagBits TextureUsageToVk(TextureUsage p_Usage)
	{
		switch (p_Usage)
		{
			using enum YUME::TextureUsage;

			case TEXTURE_SAMPLED:					return VK_IMAGE_USAGE_SAMPLED_BIT;
			case TEXTURE_COLOR_ATTACHMENT:			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case TEXTURE_DEPTH_STENCIL_ATTACHMENT:  return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

			default:
				YM_CORE_ERROR("Unknown texture format!")
				return (VkImageUsageFlagBits)0;
		}
	}

	void CopyBufferToImage(VkBuffer p_Buffer, VkImage p_Image, uint32_t p_Width, uint32_t p_Height)
	{
		YM_PROFILE_FUNCTION()

		auto commandBuffer = BeginSingleTimeCommand();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			p_Width,
			p_Height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			p_Buffer,
			p_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		EndSingleTimeCommand(commandBuffer);
	}

	VkShaderStageFlagBits ShaderTypeToVK(ShaderType p_Type)
	{
		switch (p_Type)
		{
			case ShaderType::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
			default:
				YM_CORE_ERROR("Unknown shader type")
				return (VkShaderStageFlagBits)0;
		}
	}

}

