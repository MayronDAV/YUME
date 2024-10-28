#include "YUME/yumepch.h"
#include "sub_texture.h"


namespace YUME
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& p_Texture, const glm::vec2& p_Min, const glm::vec2& p_Max)
		: m_Texture(p_Texture)
	{
		m_TexCoord[0] = { p_Min.x, p_Min.y };
		m_TexCoord[1] = { p_Max.x, p_Min.y };
		m_TexCoord[2] = { p_Max.x, p_Max.y };
		m_TexCoord[3] = { p_Min.x, p_Max.y };
	}

	Ref<SubTexture2D> SubTexture2D::Create(const SubTextureSpec& p_Spec)
	{
		float customTileSizeX = (p_Spec.Scale.x == 0.0f) ? 1.0f : p_Spec.Scale.x;
		float customTileSizeY = (p_Spec.Scale.y == 0.0f) ? 1.0f : p_Spec.Scale.y;

		glm::vec2 min = {
			(p_Spec.BySize ? p_Spec.Offset.x * p_Spec.Size.x : p_Spec.Offset.x) / p_Spec.Texture->GetWidth(),
			(p_Spec.BySize ? p_Spec.Offset.y * p_Spec.Size.y : p_Spec.Offset.y) / p_Spec.Texture->GetHeight()
		};

		glm::vec2 max = {
			(p_Spec.BySize ? (p_Spec.Offset.x + customTileSizeX) * p_Spec.Size.x : p_Spec.Offset.x + (customTileSizeX * p_Spec.Size.x)) / p_Spec.Texture->GetWidth(),
			(p_Spec.BySize ? (p_Spec.Offset.y + customTileSizeY) * p_Spec.Size.y : p_Spec.Offset.y + (customTileSizeY * p_Spec.Size.x)) / p_Spec.Texture->GetHeight()
		};

		return CreateRef<SubTexture2D>(p_Spec.Texture, min, max);
	}
}
