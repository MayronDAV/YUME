#pragma once
#include "YUME/Renderer/texture.h"
#include "Platform/Vulkan/Core/vulkan_base.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class VulkanTexture2D : public Texture2D
	{
		public:
			explicit VulkanTexture2D(const TextureSpecification& p_Spec = {});
			VulkanTexture2D(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size);
			VulkanTexture2D(VkImage p_Image, VkImageView p_ImageView, VkFormat p_Format = VK_FORMAT_R8G8B8A8_SRGB, uint32_t p_Width = 1, uint32_t p_Height = 1);
			~VulkanTexture2D() override;

			void Bind(uint32_t p_Slot = 0) const override;
			void Unbind(uint32_t p_Slot = 0) const override;

			void SetData(const unsigned char* p_Data, uint32_t p_Size) override;

			uint32_t GetWidth() const override { return m_Specification.Width; }
			uint32_t GetHeight() const override { return m_Specification.Height; }
			uint32_t GetChannels() const override { return m_Channels; }
			uint64_t GetEstimatedSize() const override { return m_Specification.Width * m_Specification.Height * m_Channels * m_BytesPerChannel; }
			const TextureSpecification& GetSpecification() const override { return m_Specification; }
			std::vector<unsigned char>& GetData() override { return m_Data; }
			VkImageView GetImageView() const { return m_TextureImageView; }
			VkSampler GetImageSampler() const { return m_TextureSampler; }
			VkImageLayout GetLayout() const { return m_TextureImageLayout; }

			void TransitionImage(VkImageLayout p_NewLayout);

			bool operator== (const Texture& p_Other) const override;

		private:
			void Init(const TextureSpecification& p_Spec);

#ifdef USE_VMA_ALLOCATOR
		void CreateImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling,
			VkImageUsageFlags p_Usage, VkImage& p_Image, VmaAllocation& p_Allocation) const;
#else
		void CreateImage(uint32_t p_Width, uint32_t p_Height, VkFormat p_Format, VkImageTiling p_Tiling,
			VkImageUsageFlags p_Usage, VkMemoryPropertyFlags p_Properties, VkImage& p_Image, VkDeviceMemory& p_ImageMemory);
		void AllocateImageMemory(VkImage& p_Image, VkDeviceMemory& p_ImageMemory, VkMemoryPropertyFlags p_Properties);
#endif
			VkImageView CreateImageView(VkImage p_Image, VkFormat p_Format);

		private:
			TextureSpecification m_Specification;

			std::vector<unsigned char> m_Data;
			std::string m_FileName;

			uint32_t m_Channels = 4;
			uint32_t m_BytesPerChannel = 1;

			VkImage m_TextureImage = VK_NULL_HANDLE;
			VkImageView m_TextureImageView = VK_NULL_HANDLE;
			VkSampler m_TextureSampler = VK_NULL_HANDLE;			

			VkFormat m_VkFormat = VK_FORMAT_R8G8B8A8_SRGB;
			VkImageLayout m_TextureImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		#ifdef USE_VMA_ALLOCATOR
			VmaAllocation m_Allocation = VK_NULL_HANDLE;
		#else
			VkDeviceMemory m_TextureImageMemory = VK_NULL_HANDLE;
		#endif

			bool m_ShouldDestroy = true;
	};
}