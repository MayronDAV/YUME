#include "YUME/yumepch.h"
#include "vulkan_texture.h"



namespace YUME
{
	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& p_Spec)
	{
	}

	VulkanTexture2D::VulkanTexture2D(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size)
	{
	}

	VulkanTexture2D::VulkanTexture2D(VkImage p_Image, VkImageView p_ImageView, VkFormat p_Format, uint32_t p_Width, uint32_t p_Height)
	{
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
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

	bool VulkanTexture2D::operator==(const Texture& other) const
	{
		return false;
	}
}
