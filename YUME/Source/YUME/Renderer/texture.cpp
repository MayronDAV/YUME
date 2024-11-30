#include "YUME/yumepch.h"
#include "texture.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_texture.h"

#include "YUME/Utils/clock.h"
#include "YUME/Utils/hash_combiner.h"
#include <glm/gtc/type_ptr.hpp>

#include "Platform/Vulkan/Utils/vulkan_utils.h"


namespace YUME
{
	struct TextureAsset
	{
		Ref<Texture> Texture		= nullptr;
		float TimeSinceLastAccessed = 0;
	};
	static std::unordered_map<uint64_t, TextureAsset> s_TextureCache;
	static const float s_CacheLifeTime = 1.0f;

	static void Combine(size_t& p_Hash, const TextureSpecification& p_Spec)
	{
		HashCombine(p_Hash, glm::value_ptr(p_Spec.BorderColor), p_Spec.BorderColorFlag);

		HashCombine(p_Hash, p_Spec.Format, p_Spec.WrapU, p_Spec.WrapV, p_Spec.WrapW, p_Spec.MinFilter, p_Spec.MagFilter);

		HashCombine(p_Hash, p_Spec.Height, p_Spec.Width, p_Spec.AnisotropyEnable, p_Spec.GenerateMips);

		HashCombine(p_Hash, p_Spec.DebugName);
	}

	void Texture::ClearCache()
	{
		YM_PROFILE_FUNCTION()

		s_TextureCache.clear();
	}

	void Texture::DeleteUnusedCache()
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

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTexture2D>(p_Spec);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec, const unsigned char* p_Data, size_t p_Size)
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

		HashCombine(hash, GetStaticType());

		Combine(hash, p_Spec);

		auto found = s_TextureCache.find(hash);
		if (found != s_TextureCache.end() && found->second.Texture)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			return found->second.Texture.As<Texture2D>();
		}

		auto texture = Create(p_Spec);
		s_TextureCache[hash] = { texture, (float)Clock::GetTime() };
		return texture;
	}
	

	Ref<TextureArray> TextureArray::Create(const TextureArraySpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTextureArray>(p_Spec);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	Ref<TextureArray> TextureArray::Create(const TextureArraySpecification& p_Spec, const uint8_t* p_Data, size_t p_Size)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTextureArray>(p_Spec, p_Data, p_Size);

		YM_CORE_ERROR("Unknown render API!")
			return nullptr;
	}

	Ref<TextureArray> TextureArray::Get(const TextureArraySpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		uint64_t hash = 0;

		HashCombine(hash, GetStaticType());
		HashCombine(hash, p_Spec.Count, p_Spec.Type);

		Combine(hash, p_Spec.Spec);

		auto found = s_TextureCache.find(hash);
		if (found != s_TextureCache.end() && found->second.Texture)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			return found->second.Texture.As<TextureArray>();
		}

		auto texture = Create(p_Spec);
		s_TextureCache[hash] = { texture, (float)Clock::GetTime() };
		return texture;
	}
}
