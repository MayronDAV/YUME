#include "YUME/yumepch.h"
#include "utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#ifdef YM_PLATFORM_LINUX
	#define STBI_NO_SIMD
#endif
#include <stb_image.h>
#include <stb_image_resize2.h>



namespace YUME::Utils
{
	// TODO?
	static constexpr uint32_t s_MaxWidth  = 4096;
	static constexpr uint32_t s_MaxHeight = 4096;

	void CreateDirectoryIfNeeded(const std::filesystem::path& p_Path)
	{
		if (!std::filesystem::exists(p_Path))
			std::filesystem::create_directories(p_Path);
	}

	uint8_t* LoadImageFromFile(const char* p_Path, uint32_t* p_Width, uint32_t* p_Height, uint32_t* p_Channels, uint32_t* p_Bytes, bool* p_IsHDR, bool p_FlipY)
	{
		YM_PROFILE_FUNCTION()

		stbi_set_flip_vertically_on_load(p_FlipY);

		int texWidth = 0, texHeight = 0, texChannels = 0;
		stbi_uc* pixels	  = nullptr;
		int sizeOfChannel = 8;
		if (stbi_is_hdr(p_Path))
		{
			sizeOfChannel = 32;
			pixels = (uint8_t*)stbi_loadf(p_Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			if (p_IsHDR) *p_IsHDR = true;
		}
		else
		{
			pixels = stbi_load(p_Path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

			if (p_IsHDR) *p_IsHDR = false;
		}

		if (!p_IsHDR && s_MaxWidth > 0 && s_MaxHeight > 0 && ((uint32_t)texWidth > s_MaxWidth || (uint32_t)texHeight > s_MaxHeight))
		{
			uint32_t texWidthOld = texWidth, texHeightOld = texHeight;
			float aspectRatio = static_cast<float>(texWidth) / static_cast<float>(texHeight);
			if ((uint32_t)texWidth > s_MaxWidth)
			{
				texWidth = s_MaxWidth;
				texHeight = static_cast<uint32_t>(s_MaxWidth / aspectRatio);
			}
			if ((uint32_t)texHeight > s_MaxHeight)
			{
				texHeight = s_MaxHeight;
				texWidth = static_cast<uint32_t>(s_MaxHeight * aspectRatio);
			}

			int resizedChannels = texChannels;
			uint8_t* resizedPixels = (stbi_uc*)malloc(texWidth * texHeight * resizedChannels);

			if (p_IsHDR)
			{
				stbir_resize_float_linear((float*)pixels, texWidthOld, texHeightOld, 0, (float*)resizedPixels, texWidth, texHeight, 0, STBIR_RGBA);
			}
			else
			{
				stbir_resize_uint8_linear(pixels, texWidthOld, texHeightOld, 0, resizedPixels, texWidth, texHeight, 0, STBIR_RGBA);
			}

			free(pixels);
			pixels = resizedPixels;
		}

		if (!pixels)
		{
			YM_CORE_ERROR("Could not load image '{}'!", p_Path);
			// Return magenta checkerboad image

			texChannels = 4;

			if (p_Width)	*p_Width	= 2;
			if (p_Height)	*p_Height	= 2;
			if (p_Bytes)	*p_Bytes	= sizeOfChannel / 8;
			if (p_Channels) *p_Channels = texChannels;

			const int32_t size = (*p_Width) * (*p_Height) * texChannels;
			uint8_t* data = new uint8_t[size];

			uint8_t datatwo[16] = {
				255, 0  , 255, 255,
				0,   0  , 0,   255,
				0,   0  , 0,   255,
				255, 0  , 255, 255
			};

			memcpy(data, datatwo, size);

			return data;
		}

		if (texChannels != 4)
			texChannels = 4;

		if (p_Width)	*p_Width	= texWidth;
		if (p_Height)	*p_Height	= texHeight;
		if (p_Bytes)	*p_Bytes	= sizeOfChannel / 8;
		if (p_Channels) *p_Channels = texChannels;

		const uint64_t size = uint64_t(texWidth) * uint64_t(texHeight) * uint64_t(texChannels) * uint64_t(sizeOfChannel / 8U);
		uint8_t* result = new uint8_t[size];
		memcpy(result, pixels, size);

		stbi_image_free(pixels);
		return result;
	}

	void GetMaxImagesSize(uint32_t* p_Width, uint32_t* p_Height)
	{
		*p_Width  = s_MaxWidth;
		*p_Height = s_MaxHeight;
	}
}
