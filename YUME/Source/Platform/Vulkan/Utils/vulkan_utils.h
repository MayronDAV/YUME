#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/definitions.h"
#include "YUME/Core/command_buffer.h"

#include <vulkan/vulkan.h>



namespace YUME::VKUtils
{
	void WaitIdle();

	std::string GetMessageSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT p_Severity);
	std::string VkResultToString(VkResult p_Result);

	std::string GetMessageType(VkDebugUtilsMessageTypeFlagsEXT p_Type);

	VkPolygonMode PolygonModeToVk(PolygonMode p_Mode);
	VkPrimitiveTopology DrawTypeToVk(DrawType p_Type);
	VkFrontFace FrontFaceToVk(FrontFace p_Face);
	VkCullModeFlags CullModeToVk(CullMode p_Mode);
	VkFormat TextureFormatToVk(TextureFormat p_Format);
	uint32_t TextureFormatChannels(TextureFormat p_Format);
	uint32_t TextureFormatBytesPerChannel(TextureFormat p_Format);
	VkFilter TextureFilterToVk(TextureFilter p_Filter);
	VkSamplerAddressMode TextureWrapToVk(TextureWrap p_Wrap);
	VkBorderColor TextureBorderColorToVk(TextureBorderColor p_BorderColor);
	VkImageUsageFlagBits TextureUsageToVk(TextureUsage p_Usage);
	VkShaderStageFlagBits ShaderTypeToVK(ShaderType p_Type);
	VkDescriptorType DescriptorTypeToVk(DescriptorType p_Type);
	VkFormat DataTypeToVkFormat(DataType p_Type);
	VkSubpassContents SubpassContentsToVk(SubpassContents p_Contents);

	void TransitionImageLayout(const VkImage& p_Image, VkFormat p_Format, VkImageLayout p_CurrentLayout, VkImageLayout p_NewLayout, CommandBuffer* p_CommandBuffer = nullptr, uint32_t p_MipLevels = 1, uint32_t p_LayerCount = 1);

	VkCommandBuffer BeginSingleTimeCommand();
	void EndSingleTimeCommand(VkCommandBuffer p_CommandBuffer);


	void CopyBufferToImage(VkBuffer p_Buffer, VkImage p_Image, uint32_t p_Width, uint32_t p_Height);

	void CopyImage(uint32_t p_Width, uint32_t p_Height, const VkImage& p_SrcImage, const VkImage& p_DestImage);
		

	bool IsPresentModeSupported(const std::vector<VkPresentModeKHR>& p_SupportedModes, VkPresentModeKHR p_PresentMode);
	VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& p_SupportedModes, bool p_Vsync);

	void SetDebugUtilsObjectName(const VkDevice& p_Device, const VkObjectType& p_ObjectType, const char* p_Name, const void* p_Handle);
	void BeginDebugUtils(VkCommandBuffer p_CommandBuffer, const char* p_Name);
	void EndDebugUtils(VkCommandBuffer p_CommandBuffer);
}