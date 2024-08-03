#include "YUME/yumepch.h"
#include "texture_importer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



namespace YUME
{

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path)
	{
		int width, height, channels;
		std::string path = p_Path;
		stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
		if (pixels == nullptr)
		{
			YM_CORE_ERROR("Failed to load texture image!")
			stbi_image_free(pixels);
			return nullptr;
		}
		uint32_t imageSize = width * height * 4 * 1;

		TextureSpecification spec{};
		spec.Width = width;
		spec.Height = height;
		spec.BorderColorFlag = TextureBorderColor::OPAQUE_BLACK_SRGB;
		switch (channels)
		{
			case 1: spec.Format = TextureFormat::R8_SRGB; break;
			case 2: spec.Format = TextureFormat::RG8_SRGB; break;
			case 3: spec.Format = TextureFormat::RGB8_SRGB; break;
			case 4: spec.Format = TextureFormat::RGBA8_SRGB; break;
			default: spec.Format = TextureFormat::RGBA8_SRGB; break;
		}
		spec.Format = TextureFormat::RGBA8_SRGB;
		spec.Usage = TextureUsage::TEXTURE_SAMPLED;

		Ref<Texture2D> texture = Texture2D::Create(spec, pixels, imageSize);
		if (!texture)
		{
			YM_CORE_ERROR("Failed to create texture!")
			stbi_image_free(pixels);
			return nullptr;
		}

		stbi_image_free(pixels);
		return texture;
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec)
	{
		TextureSpecification spec = p_Spec;
		int force_channel;
		switch (spec.Format)
		{
			case TextureFormat::R8_SRGB:    force_channel = 1; break;
			case TextureFormat::RG8_SRGB:   force_channel = 2; break;
			case TextureFormat::RGB8_SRGB:  force_channel = 3; break;
			case TextureFormat::RGBA8_SRGB: force_channel = 4; break;
			default: force_channel = 4; break;
		}
		spec.Usage = TextureUsage::TEXTURE_SAMPLED;

		int width, height, channels;
		std::string path = p_Path;
		stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, force_channel);
		if (pixels == nullptr)
		{
			YM_CORE_ERROR("Failed to load texture image!")
			stbi_image_free(pixels);
			return nullptr;
		}

		uint32_t imageSize = p_Spec.Width * p_Spec.Height * channels * 1;
		Ref<Texture2D> texture = Texture2D::Create(spec, pixels, imageSize);
		if (texture == nullptr)
		{
			YM_CORE_ERROR("Failed to create texture!")
			stbi_image_free(pixels);
			return nullptr;
		}

		stbi_image_free(pixels);
		return texture;
	}
}
