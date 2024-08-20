#include "YUME/yumepch.h"
#include "renderpass_framebuffer.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_renderpass_framebuffer.h"



namespace YUME
{
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
}
