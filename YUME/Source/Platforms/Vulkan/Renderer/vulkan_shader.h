#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/shader.h"

#include <vulkan/vulkan.h>



namespace YUME
{
	class YM_API VulkanShader : public Shader
	{
		private:
			using ShaderSource = std::unordered_map<uint32_t, std::string>;

		public:
			VulkanShader() = default;
			explicit VulkanShader(const std::string_view& p_ShaderPath);
			~VulkanShader() override;

			void Bind() override;
			void Unbind() override;

			const std::string_view& GetName() const override;

			void UploadFloat(const std::string_view& p_Name, float p_Value) override;
			void UploadInt(const std::string_view& p_Name, int p_Value) override;

		private:
			std::string ReadFile(const std::string_view& p_Filepath) const;
			ShaderSource PreProcess(const std::string& p_Source) const;

			void CompileOrGetVulkanBinaries(const ShaderSource& p_ShaderSources);
			void Reflect(uint32_t p_Stage, const std::vector<uint32_t>& p_ShaderData);

			void CreateShaderModules();

		private:
			std::string_view m_FilePath;
			std::string_view m_Name = "Untitled";
			
			std::unordered_map<uint32_t, VkShaderModule> m_ShaderModules;
			std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

			std::unordered_map<uint32_t, std::vector<uint32_t>> m_VulkanSPIRV;

			VkDevice m_Device = VK_NULL_HANDLE;
	};
}