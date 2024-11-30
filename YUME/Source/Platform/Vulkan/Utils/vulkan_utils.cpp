#include "YUME/yumepch.h"
#include "vulkan_utils.h"

#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"
#include "YUME/Core/application.h"
#include "Platform/Vulkan/Core/vulkan_command_buffer.h"

#include <utility>





namespace YUME::VKUtils
{
	void WaitIdle()
	{
		vkDeviceWaitIdle(VulkanDevice::Get().GetDevice());
	}

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
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown Message Severity!")
				return "Trace";
		}
	}

	std::string VkResultToString(VkResult p_Result)
	{
		switch (p_Result)
		{
		#define STR(r)		\
			case VK_##r:	\
				return #r

			STR(SUCCESS);
			STR(NOT_READY);
			STR(TIMEOUT);
			STR(EVENT_SET);
			STR(EVENT_RESET);
			STR(INCOMPLETE);
			STR(ERROR_OUT_OF_HOST_MEMORY);
			STR(ERROR_OUT_OF_DEVICE_MEMORY);
			STR(ERROR_INITIALIZATION_FAILED);
			STR(ERROR_DEVICE_LOST);
			STR(ERROR_MEMORY_MAP_FAILED);
			STR(ERROR_LAYER_NOT_PRESENT);
			STR(ERROR_EXTENSION_NOT_PRESENT);
			STR(ERROR_FEATURE_NOT_PRESENT);
			STR(ERROR_INCOMPATIBLE_DRIVER);
			STR(ERROR_TOO_MANY_OBJECTS);
			STR(ERROR_FORMAT_NOT_SUPPORTED);
			STR(ERROR_SURFACE_LOST_KHR);
			STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			STR(SUBOPTIMAL_KHR);
			STR(ERROR_OUT_OF_DATE_KHR);
			STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			STR(ERROR_VALIDATION_FAILED_EXT);
			STR(ERROR_INVALID_SHADER_NV);
			STR(ERROR_FRAGMENTED_POOL);
			STR(ERROR_OUT_OF_POOL_MEMORY);
			STR(ERROR_INVALID_EXTERNAL_HANDLE);
			STR(ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			STR(ERROR_FRAGMENTATION_EXT);
			STR(ERROR_NOT_PERMITTED_EXT);
			STR(ERROR_INVALID_DEVICE_ADDRESS_EXT);
			STR(ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			STR(ERROR_UNKNOWN);
			STR(RESULT_MAX_ENUM);

			#undef STR
			default:
				return "UNKNOWN_ERROR";
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
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown Message Type!")
				return "General";
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
			YM_CORE_ERROR(VULKAN_PREFIX "Unknown Draw Type!")
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
			YM_CORE_ERROR(VULKAN_PREFIX "Unknown Front Face!")
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
			YM_CORE_ERROR(VULKAN_PREFIX "Unknown Cull Mode!")
				return VK_CULL_MODE_BACK_BIT;
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
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown Polygon Mode!")
				return VK_POLYGON_MODE_FILL;
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

	static bool HasStencilComponent(VkFormat p_Format)
	{
		return  p_Format == VK_FORMAT_D16_UNORM_S8_UINT ||
				p_Format == VK_FORMAT_D24_UNORM_S8_UINT ||
				p_Format == VK_FORMAT_D32_SFLOAT_S8_UINT;
	}

	static VkPipelineStageFlags AccessFlagsToPipelineStage(VkAccessFlags p_AccessFlags, const VkPipelineStageFlags p_StageFlags)
	{
		VkPipelineStageFlags stages = 0;

		while (p_AccessFlags != 0)
		{
			VkAccessFlagBits AccessFlag = static_cast<VkAccessFlagBits>(p_AccessFlags & (~(p_AccessFlags - 1)));
			YM_CORE_ASSERT(AccessFlag != 0 && (AccessFlag & (AccessFlag - 1)) == 0, "Error");
			p_AccessFlags &= ~AccessFlag;

			switch (AccessFlag)
			{
			case VK_ACCESS_INDIRECT_COMMAND_READ_BIT:
				stages |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
				break;

			case VK_ACCESS_INDEX_READ_BIT:
				stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
				break;

			case VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT:
				stages |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
				break;

			case VK_ACCESS_UNIFORM_READ_BIT:
				stages |= p_StageFlags | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				break;

			case VK_ACCESS_INPUT_ATTACHMENT_READ_BIT:
				stages |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				break;

			case VK_ACCESS_SHADER_READ_BIT:
				stages |= p_StageFlags | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				break;

			case VK_ACCESS_SHADER_WRITE_BIT:
				stages |= p_StageFlags | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
				break;

			case VK_ACCESS_COLOR_ATTACHMENT_READ_BIT:
				stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				break;

			case VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT:
				stages |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				break;

			case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
				stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				break;

			case VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
				stages |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
				break;

			case VK_ACCESS_TRANSFER_READ_BIT:
				stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
				break;

			case VK_ACCESS_TRANSFER_WRITE_BIT:
				stages |= VK_PIPELINE_STAGE_TRANSFER_BIT;
				break;

			case VK_ACCESS_HOST_READ_BIT:
				stages |= VK_PIPELINE_STAGE_HOST_BIT;
				break;

			case VK_ACCESS_HOST_WRITE_BIT:
				stages |= VK_PIPELINE_STAGE_HOST_BIT;
				break;

			case VK_ACCESS_MEMORY_READ_BIT:
				break;

			case VK_ACCESS_MEMORY_WRITE_BIT:
				break;

			default:
				YM_CORE_ERROR("Unknown access flag");
				break;
			}
		}
		return stages;
	}

	static VkPipelineStageFlags LayoutToAccessMask(const VkImageLayout p_Layout, const bool p_IsDestination)
	{
		VkPipelineStageFlags accessMask = 0;

		switch (p_Layout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				if (p_IsDestination)
				{
					YM_CORE_ERROR("The new layout used in a transition must not be VK_IMAGE_LAYOUT_UNDEFINED.");
				}
				break;

			case VK_IMAGE_LAYOUT_GENERAL:
				accessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				accessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				accessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				accessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				accessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				if (!p_IsDestination)
				{
					accessMask = VK_ACCESS_HOST_WRITE_BIT;
				}
				else
				{
					YM_CORE_ERROR("The new layout used in a transition must not be VK_IMAGE_LAYOUT_PREINITIALIZED.");
				}
				break;

			case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
				accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
				accessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				accessMask = VK_ACCESS_2_NONE;
				break;

			default:
				YM_CORE_ERROR("Unexpected image layout");
				break;
		}

		return accessMask;
	}


	void TransitionImageLayout(const VkImage& p_Image, VkFormat p_Format, VkImageLayout p_CurrentLayout, VkImageLayout p_NewLayout, VkCommandBuffer p_CommandBuffer, uint32_t p_BaseMipLevel, uint32_t p_MipLevels, uint32_t p_Layer, uint32_t p_LayerCount)
	{
		YM_PROFILE_FUNCTION()

		VkCommandBuffer commandBuffer;
		bool singleTime	  = false;
		if (p_CommandBuffer == nullptr)
		{
			commandBuffer = BeginSingleTimeCommand();
			singleTime	  = true;
		}
		else
		{
			commandBuffer = p_CommandBuffer;
		}

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask		 = HasDepthComponent(p_Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		if (HasStencilComponent(p_Format))
			subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		subresourceRange.baseMipLevel	 = p_BaseMipLevel;
		subresourceRange.levelCount		 = p_MipLevels;
		subresourceRange.baseArrayLayer  = p_Layer;
		subresourceRange.layerCount		 = p_LayerCount;

		VkImageMemoryBarrier barrier	 = {};
		barrier.sType					 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout				 = p_CurrentLayout;
		barrier.newLayout				 = p_NewLayout;
		barrier.image					 = p_Image;
		barrier.subresourceRange		 = subresourceRange;
		barrier.srcQueueFamilyIndex		 = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex		 = VK_QUEUE_FAMILY_IGNORED;
		barrier.srcAccessMask			 = LayoutToAccessMask(p_CurrentLayout, false);
		barrier.dstAccessMask			 = LayoutToAccessMask(p_NewLayout, true);

		VkPipelineStageFlags sourceStage = 0;
		{
			if (barrier.oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			{
				sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (barrier.srcAccessMask != 0)
			{
				sourceStage = AccessFlagsToPipelineStage(barrier.srcAccessMask, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}
			else
			{
				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			}
		}

		VkPipelineStageFlags destinationStage = 0;
		{
			if (barrier.newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			{
				destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			}
			else if (barrier.dstAccessMask != 0)
			{
				destinationStage = AccessFlagsToPipelineStage(barrier.dstAccessMask, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			}
			else
			{
				destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			}
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		if (singleTime)
		{
			EndSingleTimeCommand(commandBuffer);
		}
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

		SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_COMMAND_BUFFER, "SingleTimeCommandBuffer", commandBuffer);

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
			case R32_UINT:			 return VK_FORMAT_R32_UINT;
			case R32_FLOAT:			 return VK_FORMAT_R32_SFLOAT;
			case R16_FLOAT:			 return VK_FORMAT_R16_SFLOAT;

			case RG8_SRGB:			 return VK_FORMAT_R8G8_SRGB;
			case RG32_UINT:			 return VK_FORMAT_R32G32_UINT;

			case RGB8_SRGB:			 return VK_FORMAT_R8G8B8_SRGB;

			case RGBA8_SRGB:		 return VK_FORMAT_R8G8B8A8_SRGB;
			case RGBA16_FLOAT:		 return VK_FORMAT_R16G16B16A16_SFLOAT;
			case RGBA32_FLOAT:		 return VK_FORMAT_R32G32B32A32_SFLOAT;

			case D16_UNORM:			 return VK_FORMAT_D16_UNORM;
			case D32_FLOAT:			 return VK_FORMAT_D32_SFLOAT;
			case D16_UNORM_S8_UINT:  return VK_FORMAT_D16_UNORM_S8_UINT;
			case D24_UNORM_S8_UINT:  return VK_FORMAT_D24_UNORM_S8_UINT;
			case D32_FLOAT_S8_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;

			default:
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown texture format!")
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
			case R32_UINT:
			case R32_FLOAT:
			case R16_FLOAT:
				return 1;

			case RG8_SRGB:
			case RG32_UINT:
				return 2;

			case RGB8_SRGB:
				return 3;

			case RGBA8_SRGB:
			case RGBA16_FLOAT:
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
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown texture format!")
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

			case RGBA16_FLOAT:
			case R16_FLOAT:
			case D16_UNORM:
				return 2;

			case R32_INT:
			case R32_UINT:
			case R32_FLOAT:
			case RG32_UINT:
			case RGBA32_FLOAT:
			case D32_FLOAT:
			case D24_UNORM_S8_UINT:
				return 4;

			case D16_UNORM_S8_UINT:  return 3;
			case D32_FLOAT_S8_UINT:  return 5;

			default:
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown texture format!")
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
				YM_CORE_WARN(VULKAN_PREFIX "Unknown texture filter, returning default...")
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
				YM_CORE_WARN(VULKAN_PREFIX "Unknown texture wrap, returning default...")
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
				YM_CORE_WARN(VULKAN_PREFIX "Unknown texture border color, returning default...")
				return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		}
	}

	VkImageUsageFlags TextureUsageToVk(TextureUsage p_Usage)
	{
		switch (p_Usage)
		{
			using enum YUME::TextureUsage;

			case TEXTURE_SAMPLED:					return 0;
			case TEXTURE_COLOR_ATTACHMENT:			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			case TEXTURE_DEPTH_STENCIL_ATTACHMENT:  return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			case TEXTURE_STORAGE:					return VK_IMAGE_USAGE_STORAGE_BIT;

			default:
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown texture format!")
				return 0;
		}
	}

	void CopyBufferToImage(VkBuffer p_Buffer, VkImage p_Image, uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, uint32_t p_LayerCount)
	{
		YM_PROFILE_FUNCTION()

		auto commandBuffer = BeginSingleTimeCommand();

		VkBufferImageCopy region{};
		region.bufferOffset		 = 0;
		region.bufferRowLength   = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = HasDepthComponent(p_Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (HasStencilComponent(p_Format))
			region.imageSubresource.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		region.imageSubresource.mipLevel	   = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount	   = p_LayerCount;

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

	void CopyImage(uint32_t p_Width, uint32_t p_Height, const VkImage& p_SrcImage, const VkImage& p_DestImage)
	{
		auto commandBuffer = BeginSingleTimeCommand();

		VkImageCopy copyRegion{};
		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.mipLevel = 0;
		copyRegion.srcSubresource.baseArrayLayer = 0;
		copyRegion.srcSubresource.layerCount = 1;
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.mipLevel = 0;
		copyRegion.dstSubresource.baseArrayLayer = 0;
		copyRegion.dstSubresource.layerCount = 1;
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent.width = p_Width;
		copyRegion.extent.height = p_Height;
		copyRegion.extent.depth = 1;

		// Executa a cópia da imagem
		vkCmdCopyImage(
			commandBuffer,
			p_SrcImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			p_DestImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&copyRegion);

		EndSingleTimeCommand(commandBuffer);
	}

	VkShaderStageFlagBits ShaderTypeToVK(ShaderType p_Type)
	{
		switch (p_Type)
		{
			case ShaderType::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
			default:
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown shader type")
				return (VkShaderStageFlagBits)0;
		}
	}

	VkDescriptorType DescriptorTypeToVk(DescriptorType p_Type)
	{
		switch (p_Type)
		{
			case YUME::DescriptorType::UNIFORM_BUFFER: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case YUME::DescriptorType::STORAGE_BUFFER: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case YUME::DescriptorType::IMAGE_SAMPLER:  return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case YUME::DescriptorType::STORAGE_IMAGE:  return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			default:
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown descriptor type")
				return (VkDescriptorType)0;
		}
	}

	VkFormat DataTypeToVkFormat(DataType p_Type)
	{
		switch (p_Type)
		{
			using enum YUME::DataType;

			case Float:  return VK_FORMAT_R32_SFLOAT;
			case Float2: return VK_FORMAT_R32G32_SFLOAT;
			case Float3: return VK_FORMAT_R32G32B32_SFLOAT;
			case Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;

			case Mat3:   return VK_FORMAT_R32G32B32_SFLOAT;
			case Mat4:   return VK_FORMAT_R32G32B32A32_SFLOAT;

			case UInt:   return VK_FORMAT_R32_UINT;
			case UInt2:  return VK_FORMAT_R32G32_UINT;

			case Int:    return VK_FORMAT_R32_SINT;
			case Int2:   return VK_FORMAT_R32G32_SINT;
			case Int3:   return VK_FORMAT_R32G32B32_SINT;
			case Int4:   return VK_FORMAT_R32G32B32A32_SINT;

			case Bool:   return VK_FORMAT_R8_UINT;

			default:
				YM_CORE_ASSERT(false, "Unknown DataType!")
				return (VkFormat)0;
		}
	}

	VkSubpassContents SubpassContentsToVk(SubpassContents p_Contents)
	{
		switch (p_Contents)
		{
			case YUME::SubpassContents::INLINE:				  return VK_SUBPASS_CONTENTS_INLINE;
			case YUME::SubpassContents::SECONDARY:			  return VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;
			case YUME::SubpassContents::INLINE_AND_SECONDARY: return VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR;
			default:
				YM_CORE_ERROR(VULKAN_PREFIX "Unknown subpass contents")
				return VK_SUBPASS_CONTENTS_INLINE;
		}
	}

	bool IsPresentModeSupported(const std::vector<VkPresentModeKHR>& p_SupportedModes, VkPresentModeKHR p_PresentMode)
	{
		for (const auto& mode : p_SupportedModes)
		{
			if (mode == p_PresentMode)
			{
				return true;
			}
		}
		return false;
	}


	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& p_SupportedModes, bool p_Vsync)
	{
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		if (p_Vsync)
		{
			if (IsPresentModeSupported(p_SupportedModes, VK_PRESENT_MODE_MAILBOX_KHR))
			{
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			}
			else if (IsPresentModeSupported(p_SupportedModes, VK_PRESENT_MODE_FIFO_KHR))
			{
				presentMode = VK_PRESENT_MODE_FIFO_KHR;
			}
			else
			{
				YM_CORE_ERROR(VULKAN_PREFIX "Failed to find supported presentation mode.");
			}
		}

		return presentMode;
	}

	void SetDebugUtilsObjectName(const VkDevice& p_Device, const VkObjectType& p_ObjectType, const char* p_Name, const void* p_Handle)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo;
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = p_ObjectType;
		nameInfo.pObjectName = p_Name;
		nameInfo.objectHandle = (uint64_t)p_Handle;
		nameInfo.pNext = VK_NULL_HANDLE;

		auto res = fpSetDebugUtilsObjectNameEXT(p_Device, &nameInfo);
		YM_CORE_ASSERT(res == VK_SUCCESS)
	}

	void BeginDebugUtils(VkCommandBuffer p_CommandBuffer, const char* p_Name)
	{
		VkDebugUtilsLabelEXT debugLabel{};
		debugLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		debugLabel.pLabelName = p_Name;
		fpCmdBeginDebugUtilsLabelEXT(p_CommandBuffer, &debugLabel);
	}

	void EndDebugUtils(VkCommandBuffer p_CommandBuffer)
	{
		fpCmdEndDebugUtilsLabelEXT(p_CommandBuffer);
	}

}

