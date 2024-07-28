#include "YUME/yumepch.h"
#include "utils.h"



namespace YUME
{
	void Utils::CreateDirectoryIfNeeded(const std::filesystem::path& p_Path)
	{
		if (!std::filesystem::exists(p_Path))
			std::filesystem::create_directories(p_Path);
	}
}
