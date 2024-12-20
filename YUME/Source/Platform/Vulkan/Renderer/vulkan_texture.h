#pragma once
#include "YUME/Renderer/texture.h"
#include "Platform/Vulkan/Core/vulkan_base.h"
#include "YUME/Core/command_buffer.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class VulkanTexture2D : public Texture2D
	{
		public:
			explicit VulkanTexture2D(const TextureSpecification& p_Spec);
			VulkanTexture2D(const TextureSpecification& p_Spec, const unsigned char* p_Data, size_t p_Size);
			VulkanTexture2D(VkImage p_Image, VkImageView p_ImageView, VkFormat p_Format = VK_FORMAT_R8G8B8A8_SRGB, uint32_t p_Width = 1, uint32_t p_Height = 1);
			~VulkanTexture2D() override;

			void Resize(uint32_t p_Width, uint32_t p_Height) override;

			void Bind(uint32_t p_Slot = 0) const override {};
			void Unbind(uint32_t p_Slot = 0) const override {};

			void SetData(const void* p_Data, size_t p_Size) override;

			uint32_t GetWidth() const override { return m_Specification.Width; }
			uint32_t GetHeight() const override { return m_Specification.Height; }
			uint32_t GetChannels() const override { return m_Channels; }
			uint64_t GetEstimatedSize() const override { return m_Specification.Width * m_Specification.Height * m_Channels * m_BytesPerChannel; }
			const TextureSpecification& GetSpecification() const override { return m_Specification; }
			VkImage GetImage() { return m_TextureImage; }
			VkImageView GetImageView() { return m_TextureImageView; }
			VkSampler GetImageSampler() { return m_TextureSampler; }
			VkImageLayout GetLayout() const { return m_TextureImageLayout; }

			VkImageSubresourceRange GetSubresourceRange() const;

			void TransitionImage(VkImageLayout p_NewLayout, CommandBuffer* p_CommandBuffer = nullptr);

			bool operator== (const Texture& p_Other) const override;

		private:
			void Init(const TextureSpecification& p_Spec);

		private:
			TextureSpecification m_Specification	= {};

			uint32_t m_Channels						= 4;
			uint32_t m_BytesPerChannel				= 1;

			VkImage m_TextureImage					= VK_NULL_HANDLE;
			VkImageView m_TextureImageView			= VK_NULL_HANDLE;
			VkSampler m_TextureSampler				= VK_NULL_HANDLE;

			VkFormat m_VkFormat						= VK_FORMAT_R8G8B8A8_SRGB;
			VkImageLayout m_TextureImageLayout		= VK_IMAGE_LAYOUT_UNDEFINED;

			uint32_t m_MipLevels					= 1;

		#ifdef USE_VMA_ALLOCATOR
			VmaAllocation m_Allocation				= VK_NULL_HANDLE;
		#else
			VkDeviceMemory m_TextureImageMemory		= VK_NULL_HANDLE;
		#endif

			bool m_ShouldDestroy					= true;
	};


	class VulkanTextureArray : public TextureArray
	{
		public:
			explicit VulkanTextureArray(const TextureArraySpecification& p_Spec);
			VulkanTextureArray(const TextureArraySpecification& p_Spec, const uint8_t* p_Data, size_t p_Size);
			~VulkanTextureArray() override;

			void Bind(uint32_t p_Slot = 0)	 const override {};
			void Unbind(uint32_t p_Slot = 0) const override {};

			void SetData(const void* p_Data, size_t p_Size) override;

			uint32_t GetWidth() const override { return m_Specification.Spec .Width; }
			uint32_t GetHeight() const override { return m_Specification.Spec.Height; }
			uint32_t GetChannels() const override { return m_Channels; }
			uint64_t GetEstimatedSize() const override { return m_Specification.Spec.Width * m_Specification.Spec.Height * m_Channels * m_BytesPerChannel; }
			const TextureSpecification& GetSpecification() const override { return m_Specification.Spec; }
			VkImage GetImage() { return m_TextureImage; }
			VkImageView GetImageView() { return m_TextureImageView; }
			VkSampler GetImageSampler() { return m_TextureSampler; }
			VkImageLayout GetLayout() const { return m_TextureImageLayout; }
			uint32_t GetLayerCount() const override { return m_LayerCount; }
			TextureArrayType GetArrayType() const { return m_Specification.Type; }

			VkImageSubresourceRange GetSubresourceRange() const;

			void TransitionImage(VkImageLayout p_NewLayout, CommandBuffer* p_CommandBuffer = nullptr);

			bool operator== (const Texture& p_Other) const override;

		private:
			void Init(const TextureArraySpecification& p_Spec);

		private:
			TextureArraySpecification m_Specification   = {};

			uint32_t m_Channels							= 4;
			uint32_t m_BytesPerChannel					= 1;

			VkImage m_TextureImage						= VK_NULL_HANDLE;
			VkImageView m_TextureImageView				= VK_NULL_HANDLE;
			VkSampler m_TextureSampler					= VK_NULL_HANDLE;

			VkFormat m_VkFormat							= VK_FORMAT_R8G8B8A8_SRGB;
			VkImageLayout m_TextureImageLayout			= VK_IMAGE_LAYOUT_UNDEFINED;

			uint32_t m_MipLevels						= 1;
			uint32_t m_LayerCount						= 1;
			uint8_t* m_Data								= nullptr;

	#ifdef USE_VMA_ALLOCATOR
			VmaAllocation m_Allocation					= VK_NULL_HANDLE;
	#else
			VkDeviceMemory m_TextureImageMemory			= VK_NULL_HANDLE;
	#endif
	};
}