#include "YUME/yumepch.h"
#include "descriptor_set.h"

#include "YUME/Core/engine.h"
#include "Platform/Vulkan/Renderer/vulkan_descriptor_set.h"



namespace YUME
{
	Ref<DescriptorSet> DescriptorSet::Create(const DescriptorSpec& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanDescriptorSet>(p_Spec);

		YM_CORE_ASSERT(false, "Unknown RendererAPI!")
		return nullptr;
	}
}
