#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "texture.h"


namespace YUME
{
	class YM_API TextureImporter
	{
		public:
			static Ref<Texture2D>	 LoadTexture2D(const std::string& p_Path);
			static Ref<Texture2D>	 LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec);

			static Ref<TextureArray> LoadTextureCube(const std::vector<std::string>& p_Paths);
			static Ref<TextureArray> LoadTextureCube(const std::vector<std::string>& p_Paths, const TextureSpecification& p_Spec);
	};
}