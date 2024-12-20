#pragma once

#include "texture.h"

#include <glm/glm.hpp>



namespace YUME
{
	struct YM_API SubTextureSpec
	{
		Ref<Texture2D> Texture = nullptr;

		glm::vec2 Size = { 1, 1 };

		// [true] if you want to pass the tile coord as a multiplier of the tile size
		// [false] if you want to pass the actual coord directly
		bool BySize = true;
		glm::vec2 Offset = { 0, 0 };
		glm::vec2 Scale = { 1, 1 };
	};

	class YM_API SubTexture2D
	{
		public:
			SubTexture2D(const Ref<Texture2D>& p_Texture,
				const glm::vec2& p_Min, const glm::vec2& p_Max);

			const Ref<Texture2D>& GetTexture() const { return m_Texture; }
			const glm::vec2* GetTexCoords() const { return m_TexCoord; }

			static Ref<SubTexture2D> Create(const SubTextureSpec& p_Spec);

		private:
			Ref<Texture2D> m_Texture;

			glm::vec2 m_TexCoord[4];
	};
};
