#include "YUME/yumepch.h"
#include "pipeline.h"
#include "YUME/Core/engine.h"
#include "Platform/Vulkan/Renderer/vulkan_pipeline.h"



namespace YUME
{
	Ref<Pipeline> Pipeline::Create(const PipelineCreateInfo& p_CreateInfo)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanPipeline>(p_CreateInfo);

		YM_CORE_VERIFY(false, "Unknown API!")
		return nullptr;
	}
}
