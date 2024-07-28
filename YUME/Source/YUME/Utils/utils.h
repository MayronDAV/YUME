#pragma once
#include <filesystem>

namespace YUME::Utils
{
	void CreateDirectoryIfNeeded(const std::filesystem::path& p_Path);
}