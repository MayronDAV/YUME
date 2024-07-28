#include "YUME/yumepch.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_renderer_api.h"



namespace YUME
{
	RendererAPI* RendererAPI::Create()
	{
		switch (Engine::GetAPI())
		{
			case YUME::RenderAPI::None:
				YM_CORE_VERIFY(false, "Renderer API type, must to be defined!")
				break;
			case YUME::RenderAPI::Vulkan: return new VulkanRendererAPI();
		}

		YM_CORE_VERIFY(false, "Currently only support Vulkan")
		return nullptr;
	}
}
