#include "YUME/yumepch.h"
#include "renderpass_framebuffer.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_renderpass_framebuffer.h"

#include "YUME/Utils/clock.h"
#include "YUME/Utils/hash_combiner.h"



namespace YUME
{
	struct FramebufferAsset
	{
		Ref<RenderPassFramebuffer> Framebuffer;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, FramebufferAsset> s_FramebufferCache;
	static const float s_CacheLifeTime = 1.0f;

	Ref<RenderPassFramebuffer> RenderPassFramebuffer::Create(const RenderPassFramebufferSpec& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
		{
			return CreateRef<VulkanRenderPassFramebuffer>(p_Spec);
		}

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	// TODO: Change this when add UUID
	Ref<RenderPassFramebuffer> RenderPassFramebuffer::Get(const RenderPassFramebufferSpec& p_Spec)
	{
		YM_PROFILE_FUNCTION();

		uint64_t hash = 0;
		HashCombine(hash, p_Spec.Width, p_Spec.Height);

		for (const auto& texture : p_Spec.Attachments)
		{
			if (texture)
			{
				HashCombine(hash, texture->Handle);
			}
		}

		HashCombine(hash, p_Spec.RenderPass.get());

		auto found = s_FramebufferCache.find(hash);
		if (found != s_FramebufferCache.end() && found->second.Framebuffer)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			return found->second.Framebuffer;
		}

		auto framebuffer = Create(p_Spec);
		s_FramebufferCache[hash] = { framebuffer, (float)Clock::GetTime() };
		return framebuffer;
	}

	void RenderPassFramebuffer::ClearCache()
	{
		YM_PROFILE_FUNCTION()

		s_FramebufferCache.clear();
	}

	void RenderPassFramebuffer::DeleteUnusedCache()
	{
		YM_PROFILE_FUNCTION()

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_FramebufferCache)
		{
			if (value.Framebuffer && (Clock::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_FramebufferCache[keysToDelete[i]].Framebuffer = nullptr;
			s_FramebufferCache.erase(keysToDelete[i]);
		}
	}
}
