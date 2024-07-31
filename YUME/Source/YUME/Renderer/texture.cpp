#include "YUME/yumepch.h"
#include "texture.h"
#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_texture.h"



namespace YUME
{
	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec)
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTexture2D>(p_Spec);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size)
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return CreateRef<VulkanTexture2D>(p_Spec, p_Data, p_Size);

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}
}
