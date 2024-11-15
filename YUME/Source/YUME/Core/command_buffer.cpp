#include "YUME/yumepch.h"
#include "command_buffer.h"
#include "YUME/Core/engine.h"
#include "Platform/Vulkan/Core/vulkan_command_buffer.h"
#include "YUME/Renderer/pipeline.h"



namespace YUME
{
	Unique<CommandBuffer> CommandBuffer::Create(const std::string& p_DebugName)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateUnique<VulkanCommandBuffer>(p_DebugName);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}
}
