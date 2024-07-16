#include "YUME/yumepch.h"
#include "shader.h"

#include "YUME/Core/yume_config.h"

#include "Platforms/Vulkan/Renderer/vulkan_shader.h"



namespace YUME
{
	Ref<Shader> Shader::Create(const std::string& p_ShaderPath)
	{
		switch (s_RenderAPI)
		{
			case RenderAPI::Vulkan: return CreateRef<VulkanShader>(p_ShaderPath);
		}

		YM_CORE_ERROR("Unknown render API!")
		return Ref<Shader>();
	}

}