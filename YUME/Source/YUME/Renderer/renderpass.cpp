#include "YUME/yumepch.h"
#include "renderpass.h"
#include "YUME/Core/engine.h"
#include "YUME/Renderer/renderpass_framebuffer.h"

#include "Platform/Vulkan/Renderer/vulkan_renderpass.h"

#include "YUME/Utils/clock.h"
#include "YUME/Utils/hash_combiner.h"




namespace YUME
{
	struct RenderPassAsset
	{
		Ref<RenderPass> RenderPass;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, RenderPassAsset> s_RenderPassCache;
	static const float s_CacheLifeTime = 0.1f;

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
		{
			return CreateRef<VulkanRenderPass>(p_Spec);
		}

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	// TODO: Change this when add UUID
	Ref<RenderPass> RenderPass::Get(const RenderPassSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		uint64_t hash = 0;
		HashCombine(hash, p_Spec.Samples, p_Spec.ClearEnable, p_Spec.SwapchainTarget);

		for (const auto& texture : p_Spec.Attachments)
		{
			if (texture)
			{
				HashCombine(hash, texture->Handle);
			}
		}

		if (p_Spec.ResolveTexture)
		{
			HashCombine(hash, p_Spec.ResolveTexture->Handle);
		}

		auto found = s_RenderPassCache.find(hash);
		if (found != s_RenderPassCache.end() && found->second.RenderPass)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			return found->second.RenderPass;
		}

		auto renderPass = Create(p_Spec);
		s_RenderPassCache[hash] = { renderPass, (float)Clock::GetTime() };
		return renderPass;
	}

	void RenderPass::ClearCache()
	{
		YM_PROFILE_FUNCTION()

		s_RenderPassCache.clear();
	}

	void RenderPass::DeleteUnusedCache()
	{
		YM_PROFILE_FUNCTION()

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_RenderPassCache)
		{
			if (value.RenderPass && (Clock::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_RenderPassCache[keysToDelete[i]].RenderPass = nullptr;
			s_RenderPassCache.erase(keysToDelete[i]);
		}
	}
}
