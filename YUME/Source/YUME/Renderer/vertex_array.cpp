#include "YUME/yumepch.h"
#include "vertex_array.h"

#include <YUME/Core/engine.h>
#include "Platform/Vulkan/Renderer/vulkan_vertex_array.h"



namespace YUME
{
	Ref<VertexArray> VertexArray::Create()
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanVertexArray>();

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}
}
