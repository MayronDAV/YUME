#pragma once
#include "YUME/Core/base.h"
#include "texture.h"


namespace YUME
{
	class YM_API TextureImporter
	{
		public:
			static Ref<Texture2D> LoadTexture2D(const std::string& p_Path);
			static Ref<Texture2D> LoadTexture2D(const std::string& p_Path, const TextureSpecification& p_Spec);
	};
}