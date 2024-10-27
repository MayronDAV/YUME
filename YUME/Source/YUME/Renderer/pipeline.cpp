#include "YUME/yumepch.h"
#include "pipeline.h"
#include "YUME/Core/engine.h"
#include "Platform/Vulkan/Renderer/vulkan_pipeline.h"

#include "YUME/Utils/clock.h"
#include "YUME/Utils/hash_combiner.h"

#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"




namespace YUME
{
	struct PipelineAsset
	{
		Ref<Pipeline> Pipeline;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, PipelineAsset> s_PipelineCache;
	static const float s_CacheLifeTime = 0.1f;


	Ref<Pipeline> Pipeline::Create(const PipelineCreateInfo& p_CreateInfo)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanPipeline>(p_CreateInfo);

		YM_CORE_VERIFY(false, "Unknown API!")
		return nullptr;
	}

	// TODO: Change this when add UUID
	Ref<Pipeline> Pipeline::Get(const PipelineCreateInfo& p_CreateInfo)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(p_CreateInfo.Shader != nullptr)

		uint64_t hash = 0;
		HashCombine(hash, p_CreateInfo.Shader.get(), p_CreateInfo.CullMode, (uint32_t)p_CreateInfo.DrawType, p_CreateInfo.LineWidth,
			(uint32_t)p_CreateInfo.PolygonMode, p_CreateInfo.TransparencyEnabled, p_CreateInfo.DepthTest, p_CreateInfo.DepthWrite);

		for (const auto& texture : p_CreateInfo.ColorTargets)
		{
			if (texture)
			{
				HashCombine(hash, texture->Handle);
			}
		}

		if (p_CreateInfo.DepthTarget)
		{
			HashCombine(hash, p_CreateInfo.DepthTarget->Handle);
		}

		HashCombine(hash, p_CreateInfo.ClearTargets);
		HashCombine(hash, p_CreateInfo.SwapchainTarget);

		auto found = s_PipelineCache.find(hash);
		if (found != s_PipelineCache.end() && found->second.Pipeline)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			return found->second.Pipeline;
		}

		Ref<Pipeline> pipeline = Create(p_CreateInfo);
		s_PipelineCache[hash] = { pipeline, (float)Clock::GetTime() };
		return pipeline;
	}

	void Pipeline::ClearCache()
	{
		YM_PROFILE_FUNCTION()

		s_PipelineCache.clear();
	}

	void Pipeline::DeleteUnusedCache()
	{
		YM_PROFILE_FUNCTION()

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_PipelineCache)
		{
			if (value.Pipeline && (Clock::GetTime() - value.TimeSinceLastAccessed) > s_CacheLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			s_PipelineCache[keysToDelete[i]].Pipeline = nullptr;
			s_PipelineCache.erase(keysToDelete[i]);
		}
	}

	uint32_t Pipeline::GetWidth()
	{
		YM_PROFILE_FUNCTION()

		if (m_CreateInfo.SwapchainTarget)
		{
			if (Engine::GetAPI() == RenderAPI::Vulkan)
				return VulkanSwapchain::Get().GetExtent2D().width;
		}

		if (m_CreateInfo.ColorTargets[0])
		{
			return m_CreateInfo.ColorTargets[0]->GetWidth();
		}

		if (m_CreateInfo.DepthTarget)
			return m_CreateInfo.DepthTarget->GetWidth();


		YM_CORE_WARN("Invalid pipeline width")
		return 0;
	}

	uint32_t Pipeline::GetHeight()
	{
		YM_PROFILE_FUNCTION()

		if (m_CreateInfo.SwapchainTarget)
		{
			if (Engine::GetAPI() == RenderAPI::Vulkan)
				return VulkanSwapchain::Get().GetExtent2D().height;
		}

		if (m_CreateInfo.ColorTargets[0])
		{
			return m_CreateInfo.ColorTargets[0]->GetHeight();
		}

		if (m_CreateInfo.DepthTarget)
			return m_CreateInfo.DepthTarget->GetHeight();


		YM_CORE_WARN("Invalid pipeline width")
		return 0;
	}
}
