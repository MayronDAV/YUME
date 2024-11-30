#pragma once
#include "YUME/Core/definitions.h"

#include <filesystem>


namespace YUME::Utils
{
	void CreateDirectoryIfNeeded(const std::filesystem::path& p_Path);

	uint8_t* LoadImageFromFile(const char* p_Path, uint32_t* p_Width, uint32_t* p_Height, uint32_t* p_Channels, uint32_t* p_Bytes, bool* p_IsHDR = nullptr, bool p_FlipY = false);

	void GetMaxImagesSize(uint32_t* p_Width, uint32_t* p_Height);
}