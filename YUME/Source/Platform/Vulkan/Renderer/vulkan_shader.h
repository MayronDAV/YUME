#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/shader.h"
#include "vulkan_pipeline.h"
#include "YUME/Core/definitions.h"

// Lib
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>




namespace YUME
{
	class VulkanShader : public Shader
	{
		private:
			using ShaderSource = std::unordered_map<ShaderType, std::string>;

		public:
			VulkanShader() = default;
			explicit VulkanShader(const std::string_view& p_ShaderPath);
			~VulkanShader() override;

			void CleanUp();

			void Bind() override;
			void Unbind() override;

			const std::string_view& GetName() const override;

			void SetLayout(const InputLayout& p_Layout) override;

			VkPipelineLayout& GetLayout() { return m_PipelineLayout; }
			const std::vector<DescriptorInfo>& GetDescriptorsInfo(int p_Set) { return m_DescriptorsInfo[p_Set];  }
			VkDescriptorSetLayout& GetDescriptorSetLayout(int p_Set) { return m_DescriptorSetLayouts[p_Set]; }
			std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; }

			const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescription() const { return m_AttributeDescs; }
			const std::vector<VkVertexInputBindingDescription>& GetBindingDescription() const { return m_BindingDescs; }

			void SetPushValue(const std::string& p_Name, void* p_Value) override;
			void BindPushConstants() const override;

		private:
			std::string ReadFile(const std::string_view& p_Filepath) const;
			std::string ProcessIncludeFiles(const std::string& p_Code) const;
			ShaderSource PreProcess(const std::string& p_Source) const;

			void CompileOrGetVulkanBinaries(const ShaderSource& p_ShaderSources);
			void Reflect(ShaderType p_Stage, const std::vector<uint32_t>& p_ShaderData);

			void CreateShaderModules();
			void CreatePipelineLayout();

		private:
			std::string_view m_FilePath;
			std::string_view m_Name = "Untitled";
			
			std::unordered_map<ShaderType, VkShaderModule> m_ShaderModules;
			std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

			std::unordered_map<ShaderType, std::vector<uint32_t>> m_VulkanSPIRV;

			uint32_t m_VertexBufferLocation = 0;
			uint32_t m_VertexBufferBinding = 0;

			std::vector<VkVertexInputAttributeDescription> m_AttributeDescs;
			std::vector<VkVertexInputBindingDescription> m_BindingDescs;
			VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

			VkShaderStageFlags m_Stages = 0;

			std::vector<PushConstant> m_PushConstants;

			std::unordered_map<uint32_t, std::vector<DescriptorInfo>> m_DescriptorsInfo;

			std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_DescriptorSetLayoutBindings;
			std::unordered_map<uint32_t, VkDescriptorSetLayout> m_DescriptorSetLayouts;
	};
}