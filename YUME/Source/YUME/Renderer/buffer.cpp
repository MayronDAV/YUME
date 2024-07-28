#include "YUME/yumepch.h"
#include "buffer.h"

#include <YUME/Core/engine.h>
#include "Platform/Vulkan/Renderer/vulkan_buffer.h"



namespace YUME
{
	Ref<VertexBuffer> VertexBuffer::Create(uint64_t p_SizeBytes)
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanVertexBuffer>(p_SizeBytes);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* p_Vertices, uint64_t p_SizeBytes)
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanVertexBuffer>(p_Vertices, p_SizeBytes);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}


	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* p_Indices, uint32_t p_Count)
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanIndexBuffer>(p_Indices, p_Count);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}

}
