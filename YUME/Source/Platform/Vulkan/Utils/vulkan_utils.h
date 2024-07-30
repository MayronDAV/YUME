#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/definitions.h"

#include <vulkan/vulkan.h>



namespace YUME::Utils
{
	std::string GetMessageSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity);

	std::string GetMessageType(VkDebugUtilsMessageTypeFlagsEXT p_Type);

	VkPolygonMode PolygonModeToVk(PolygonMode p_Mode);

	VkPrimitiveTopology DrawTypeToVk(DrawType p_Type);

	VkFrontFace FrontFaceToVk(FrontFace p_Face);

	VkCullModeFlags CullModeToVk(CullMode p_Mode);

	void TransitionImageLayout(VkImage p_Image, VkImageLayout p_CurrentLayout, VkImageLayout p_NewLayout);

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags p_AspectMask);

	VkRenderingAttachmentInfo AttachmentInfo(VkImageView p_View, VkClearValue* p_Clear, VkImageLayout p_Layout);
}