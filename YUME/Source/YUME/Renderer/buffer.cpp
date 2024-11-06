#include "YUME/yumepch.h"
#include "buffer.h"

#include <YUME/Core/engine.h>
#include "Platform/Vulkan/Renderer/vulkan_buffer.h"



namespace YUME
{

	Ref<VertexBuffer> VertexBuffer::Create(const void* p_Data, uint64_t p_SizeBytes, BufferUsage p_Usage)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanVertexBuffer>(p_Data, p_SizeBytes, p_Usage);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}


	Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* p_Indices, uint32_t p_Count)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanIndexBuffer>(p_Indices, p_Count);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

}
