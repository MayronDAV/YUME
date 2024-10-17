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
		float customTileSizeX = (p_Spec.CustomTileSize.x == 0.0f) ? 1.0f : p_Spec.CustomTileSize.x;
		float customTileSizeY = (p_Spec.CustomTileSize.y == 0.0f) ? 1.0f : p_Spec.CustomTileSize.y;

		glm::vec2 min = {
			(p_Spec.ByTileSize ? p_Spec.TileCoord.x * p_Spec.TileSize.x : p_Spec.TileCoord.x) / p_Spec.Texture->GetWidth(),
			(p_Spec.ByTileSize ? p_Spec.TileCoord.y * p_Spec.TileSize.y : p_Spec.TileCoord.y) / p_Spec.Texture->GetHeight()
		};

		glm::vec2 max = {
			(p_Spec.ByTileSize ? (p_Spec.TileCoord.x + customTileSizeX) * p_Spec.TileSize.x : p_Spec.TileCoord.x + (customTileSizeX * p_Spec.TileSize.x)) / p_Spec.Texture->GetWidth(),
			(p_Spec.ByTileSize ? (p_Spec.TileCoord.y + customTileSizeY) * p_Spec.TileSize.y : p_Spec.TileCoord.y + (customTileSizeY * p_Spec.TileSize.x)) / p_Spec.Texture->GetHeight()
		};

		return CreateRef<SubTexture2D>(p_Spec.Texture, min, max);
	}
}
