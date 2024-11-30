#include "YUME/yumepch.h"
#include "texture_importer.h"
#include "YUME/Utils/utils.h"



namespace YUME
{

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path)
	{
		YM_PROFILE_FUNCTION()

		TextureSpecification spec   = {};
		spec.BorderColorFlag		= TextureBorderColor::OPAQUE_BLACK_SRGB;
		spec.WrapU					= TextureWrap::CLAMP_TO_EDGE;
		spec.WrapV					= TextureWrap::CLAMP_TO_EDGE;
		spec.MinFilter				= TextureFilter::LINEAR;
		spec.MagFilter				= TextureFilter::LINEAR;
		spec.AnisotropyEnable		= true;
		spec.GenerateMips			= true;
		spec.DebugName				= std::filesystem::path(p_Path).stem().string();

		return LoadTexture2D(p_Path, spec);
	}

	Ref<Texture2D> TextureImporter::LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		uint32_t width, height, channels = 4, bytes = 1;
		bool isHDR					= false;
		std::string path			= p_Path;
		uint8_t* data				= Utils::LoadImageFromFile(p_Path.c_str(), &width, &height, &channels, &bytes, &isHDR);

		TextureSpecification spec   = p_Spec;
		spec.Width					= width;
		spec.Height					= height;
		spec.Format					= (isHDR) ? TextureFormat::RGBA32_FLOAT : TextureFormat::RGBA8_SRGB;
		spec.Usage					= TextureUsage::TEXTURE_SAMPLED;

		uint64_t imageSize			= uint64_t(width) * uint64_t(height) * uint64_t(channels) * uint64_t(bytes);
		Ref<Texture2D> texture		= Texture2D::Create(spec, data, imageSize);
		if (!texture)
		{
			YM_CORE_ERROR("Failed to create texture!")
			free(data);
			return nullptr;
		}

		free(data);
		return texture;
	}

	Ref<TextureArray> TextureImporter::LoadTextureCube(const std::vector<std::string>& p_Paths)
	{
		YM_PROFILE_FUNCTION()

		TextureSpecification spec	= {};
		spec.Usage					= TextureUsage::TEXTURE_SAMPLED;
		spec.BorderColorFlag		= TextureBorderColor::OPAQUE_BLACK_SRGB;
		spec.WrapU					= TextureWrap::CLAMP_TO_EDGE;
		spec.WrapV					= TextureWrap::CLAMP_TO_EDGE;
		spec.WrapW					= TextureWrap::CLAMP_TO_EDGE;
		spec.MinFilter				= TextureFilter::LINEAR;
		spec.MagFilter				= TextureFilter::LINEAR;
		spec.AnisotropyEnable		= true;
		spec.GenerateMips			= true;
		spec.DebugName				= "TextureCubeMap";

		return LoadTextureCube(p_Paths, spec);
	}

	static const uint8_t s_CUBEMAP_SIZE = 6;

	Ref<TextureArray> TextureImporter::LoadTextureCube(const std::vector<std::string>& p_Paths, const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		// face order -> Right, Left, Top, Down, Front, Back

		TextureArraySpecification spec	= {};
		spec.Spec						= p_Spec;
		spec.Spec.Usage					= TextureUsage::TEXTURE_SAMPLED;
		spec.Count						= s_CUBEMAP_SIZE;
		spec.Type						= TextureArrayType::CubeMap;


		uint32_t srcWidth, srcHeight, srcChannels = 4, srcBytes = 1;
		uint8_t** cubeTextureData = new uint8_t * [s_CUBEMAP_SIZE];
		for (int face = 0; face < s_CUBEMAP_SIZE; face++)
			cubeTextureData[face] = nullptr;

		uint32_t faceWidths[s_CUBEMAP_SIZE]  = {};
		uint32_t faceHeights[s_CUBEMAP_SIZE] = {};
		uint32_t channels					 = 4;
		uint32_t bytes						 = 1;
		size_t size							 = 0;
		bool isHDR							 = false;

		for (int face = 0; face < s_CUBEMAP_SIZE; face++)
		{
			if (face >= p_Paths.size())
				break;

			auto path			  = p_Paths[face];
			uint8_t* data		  = Utils::LoadImageFromFile(path.c_str(), &srcWidth, &srcHeight, &srcChannels, &srcBytes, &isHDR);

			uint32_t faceWidth	  = srcWidth;
			uint32_t faceHeight	  = srcHeight;

			if (face == 0)
			{
				spec.Spec.Width   = faceWidth;
				spec.Spec.Height  = faceHeight;
				spec.Spec.Format  = (isHDR) ? TextureFormat::RGBA32_FLOAT : TextureFormat::RGBA8_SRGB;
				channels		  = srcChannels;
				bytes			  = srcBytes;
			}

			uint32_t stride		  = channels * bytes;

			faceWidths[face]	  = faceWidth;
			faceHeights[face]	  = faceHeight;

			cubeTextureData[face] = new uint8_t[faceWidth * faceHeight * stride];
			size				 += uint64_t(faceWidth) * uint64_t(faceHeight) * uint64_t(stride);

			for (uint32_t y = 0; y < faceHeight; y++)
			{
				for (uint32_t x = 0; x < faceWidth; x++)
				{
					cubeTextureData[face][(x + y * faceWidth) * stride + 0] = data[(x + y * faceWidth) * stride + 0];
					cubeTextureData[face][(x + y * faceWidth) * stride + 1] = data[(x + y * faceWidth) * stride + 1];
					cubeTextureData[face][(x + y * faceWidth) * stride + 2] = data[(x + y * faceWidth) * stride + 2];
					if (stride >= 4)
						cubeTextureData[face][(x + y * faceWidth) * stride + 3] = data[(x + y * faceWidth) * stride + 3];
				}
			}
			delete[] data;
		}

		std::vector<uint8_t> allData;
		allData.resize(size);
		uint32_t pointeroffset = 0;

		for (uint32_t face = 0; face < s_CUBEMAP_SIZE; face++)
		{
			uint32_t currentSize = faceWidths[face] * faceHeights[face] * channels * bytes;
			memcpy(allData.data() + pointeroffset, cubeTextureData[face], currentSize);
			pointeroffset += currentSize;
		}

		for (uint32_t face = 0; face < s_CUBEMAP_SIZE; face++)
		{
			delete[] cubeTextureData[face];
		}
		delete[] cubeTextureData;

		return TextureArray::Create(spec, allData.data(), allData.size());
	}
}
