#include "YUME/yumepch.h"
#include "storage_buffer.h"
#include "YUME/Core/engine.h"
#include "Platform/Vulkan/Renderer/vulkan_storage_buffer.h"


namespace YUME
{

	Ref<StorageBuffer> StorageBuffer::Create(size_t p_SizeBytes)
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanStorageBuffer>(p_SizeBytes);
		else
		{
			YM_CORE_ERROR("Unknown render api!")
				return nullptr;
		}
	}

}