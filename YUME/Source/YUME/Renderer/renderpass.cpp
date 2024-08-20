#include "YUME/yumepch.h"
#include "renderpass.h"
#include "YUME/Core/engine.h"
#include "YUME/Renderer/renderpass_framebuffer.h"

#include "Platform/Vulkan/Renderer/vulkan_renderpass.h"



namespace YUME
{
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
}
