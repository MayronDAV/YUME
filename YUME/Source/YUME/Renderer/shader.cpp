#include "YUME/yumepch.h"
#include "shader.h"

#include "YUME/Core/engine.h"

#include "Platform/Vulkan/Renderer/vulkan_shader.h"



namespace YUME
{
	Ref<Shader> Shader::Create(const std::string& p_ShaderPath)
	{
		YM_PROFILE_FUNCTION()

		switch (Engine::GetAPI())
		{
			case RenderAPI::Vulkan: return CreateRef<VulkanShader>(p_ShaderPath);
		}

		YM_CORE_ERROR("Unknown render API!")
		return nullptr;
	}

}