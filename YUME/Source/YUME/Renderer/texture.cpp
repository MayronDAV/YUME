#include "YUME/yumepch.h"
#include "texture.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_texture.h"

#include "YUME/Utils/clock.h"
#include "YUME/Utils/hash_combiner.h"
#include <glm/gtc/type_ptr.hpp>



namespace YUME
{
	struct TextureAsset
	{
		Ref<Texture2D> Texture;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, TextureAsset> s_TextureCache;
	static const float s_CacheLifeTime = 1.0f;

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTexture2D>(p_Spec);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTexture2D>(p_Spec, p_Data, p_Size);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}


	Ref<Texture2D> Texture2D::Get(const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		uint64_t hash = 0;
		HashCombine(hash, glm::value_ptr(p_Spec.BorderColor), p_Spec.BorderColorFlag);

		HashCombine(hash, p_Spec.Format, p_Spec.WrapU, p_Spec.WrapV, p_Spec.WrapW, p_Spec.MinFilter, p_Spec.MagFilter);

		HashCombine(hash, p_Spec.Height, p_Spec.Width);

		auto found = s_TextureCache.find(hash);
		if (found != s_TextureCache.end() && found->second.Texture)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			return found->second.Texture;
		}

		auto texture = Create(p_Spec);
		s_TextureCache[hash] = { texture, (float)Clock::GetTime() };
		return texture;
	}

	void Texture2D::ClearCache()
	{
		YM_PROFILE_FUNCTION()

		s_TextureCache.clear();
	}

	void Texture2D::DeleteUnusedCache()
	{
		YM_PROFILE_FUNCTION()

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_TextureCache)
		{
			if (value.Texture && (Clock::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_TextureCache[keysToDelete[i]].Texture = nullptr;
			s_TextureCache.erase(keysToDelete[i]);
		}
	}
}
