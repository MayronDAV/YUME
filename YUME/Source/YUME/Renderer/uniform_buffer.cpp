#include "YUME/yumepch.h"
#include "uniform_buffer.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_uniform_buffer.h"



namespace YUME
{
	Ref<UniformBuffer> UniformBuffer::Create(uint32_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanUniformBuffer>(p_SizeBytes);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	Ref<UniformBuffer> UniformBuffer::Create(const void* p_Data, uint32_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanUniformBuffer>(p_Data, p_SizeBytes);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}
}
