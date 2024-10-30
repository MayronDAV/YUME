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

	void TransitionImageLayout(const VkImage& p_Image, VkFormat p_Format, VkImageLayout p_CurrentLayout, VkImageLayout p_NewLayout, bool p_UseSingleTime = true, uint32_t p_MipLevels = 1);

	VkRenderingAttachmentInfo AttachmentInfo(VkImageView p_View, VkClearValue* p_Clear, VkImageLayout p_Layout);

	VkCommandBuffer BeginSingleTimeCommand();
	void EndSingleTimeCommand(VkCommandBuffer p_CommandBuffer);

	VkFormat TextureFormatToVk(TextureFormat p_Format);
	uint32_t TextureFormatChannels(TextureFormat p_Format);
	uint32_t TextureFormatBytesPerChannel(TextureFormat p_Format);
	VkFilter TextureFilterToVk(TextureFilter p_Filter);
	VkSamplerAddressMode TextureWrapToVk(TextureWrap p_Wrap);
	VkBorderColor TextureBorderColorToVk(TextureBorderColor p_BorderColor);
	VkImageUsageFlagBits TextureUsageToVk(TextureUsage p_Usage);

	void CopyBufferToImage(VkBuffer p_Buffer, VkImage p_Image, uint32_t p_Width, uint32_t p_Height);

	void CopyImage(uint32_t p_Width, uint32_t p_Height, const VkImage& p_SrcImage, const VkImage& p_DestImage);
		
	VkShaderStageFlagBits ShaderTypeToVK(ShaderType p_Type);

	VkDescriptorType DescriptorTypeToVk(DescriptorType p_Type);
}