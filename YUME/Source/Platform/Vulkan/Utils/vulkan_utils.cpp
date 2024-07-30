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

	void TransitionImageLayout(VkImage p_Image, VkImageLayout p_CurrentLayout, VkImageLayout p_NewLayout)
	{
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

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = p_CurrentLayout;
		barrier.newLayout = p_NewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = p_Image;

		VkImageAspectFlags aspectMask = (p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
			p_NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
			: VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange = ImageSubresourceRange(aspectMask);

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		if (p_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_NewLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (p_CurrentLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && p_NewLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = 0;
			srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else {
			YM_CORE_ASSERT(false, "Unsupported layout transition!")
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		auto& graphicQueue = VulkanDevice::Get().GetGraphicQueue();

		vkQueueSubmit(graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}


	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags p_AspectMask)
	{
		VkImageSubresourceRange subImage{};
		subImage.aspectMask = p_AspectMask;
		subImage.baseMipLevel = 0;
		subImage.levelCount = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

		return subImage;
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

}

