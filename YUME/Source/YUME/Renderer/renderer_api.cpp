#include "YUME/yumepch.h"
#include "YUME/Renderer/renderer_api.h"
#include "YUME/Core/yume_config.h"

#include "Platforms/Vulkan/Renderer/vulkan_renderer_api.h"



namespace YUME
{
	RendererAPI* RendererAPI::Create()
	{
		switch (s_RenderAPI)
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
