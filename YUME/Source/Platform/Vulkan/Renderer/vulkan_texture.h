#pragma once
#include "YUME/Renderer/texture.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class YM_API VulkanTexture2D : public Texture2D
	{
		public:
			VulkanTexture2D(const TextureSpecification& p_Spec = {});
			VulkanTexture2D(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size);
			VulkanTexture2D(VkImage p_Image, VkImageView p_ImageView, VkFormat p_Format, uint32_t p_Width, uint32_t p_Height);
			~VulkanTexture2D() override;

			uint32_t GetWidth() const override { return m_Specification.Width; }
			uint32_t GetHeight() const override { return m_Specification.Height; }
			uint32_t GetChannels() const override { return m_Channels; }

			uint64_t GetEstimatedSize() const override { return m_Data.size() * m_Channels * m_BitsPerChannel; }

			const TextureSpecification& GetSpecification() const override { return m_Specification; }

			std::vector<unsigned char>& GetData() override { return m_Data; }

			void SetData(const unsigned char* p_Data, uint32_t p_Size) override;

			void Bind(uint32_t p_Slot = 0) const override;
			void Unbind(uint32_t p_Slot = 0) const override;

			bool operator== (const Texture& other) const override;

		private:
			TextureSpecification m_Specification;

			std::vector<unsigned char> m_Data;
			std::string m_FileName = "YMSource";

			uint32_t m_Channels = 4;
			uint32_t m_BitsPerChannel = 1;

	};
}