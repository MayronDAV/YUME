#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/shader.h"
#include "YUME/Renderer/vertex_array.h"
#include "vulkan_pipeline.h"
#include "YUME/Core/definitions.h"

// Lib
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>




namespace YUME
{

	class YM_API VulkanShader : public Shader
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

			VkPipelineLayout& GetLayout() { return m_PipelineLayout; }
			std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; }

			void AddVertexArray(const Ref<VertexArray>& p_VertexArray) override
			{
				if (auto pipeline = m_Pipeline.lock())
					dynamic_cast<VulkanPipeline*>(pipeline.get())->AddVertexArray(p_VertexArray);
				else
				{
					YM_CORE_ERROR("Pipeline has deleted!")
				}
			}
			void SetPipeline(const Ref<Pipeline>& p_Pipeline) override { YM_CORE_VERIFY(p_Pipeline->GetShader() == this)  m_Pipeline = p_Pipeline; }

			void UploadFloat(const std::string& p_Name, float p_Value) override;
			void UploadFloat3(const std::string& p_Name, const glm::vec3& p_Value) override;
			void UploadFloat4(const std::string& p_Name, const glm::vec4& p_Value) override;
			void UploadMat4(const std::string& p_Name, const glm::mat4& p_Value) override;
			void UploadInt(const std::string& p_Name, int p_Value) override;

			void UploadUniformBuffer(const Ref<UniformBuffer>& p_UniformBuffer) override;

		private:
			std::string ReadFile(const std::string_view& p_Filepath) const;
			std::string ProcessIncludeFiles(const std::string& p_Code) const;
			ShaderSource PreProcess(const std::string& p_Source) const;

			void CompileOrGetVulkanBinaries(const ShaderSource& p_ShaderSources);
			void Reflect(ShaderType p_Stage, const std::vector<uint32_t>& p_ShaderData);

			void CreateShaderModules();
			void CreatePipelineLayout();

			void CreateDescriptorSet();

			bool UploadPushConstantData(const std::string& p_Name, const void* p_Data, size_t p_SizeBytes);

		private:
			std::string_view m_FilePath;
			std::string_view m_Name = "Untitled";
			
			std::unordered_map<ShaderType, VkShaderModule> m_ShaderModules;
			std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;

			std::unordered_map<ShaderType, std::vector<uint32_t>> m_VulkanSPIRV;

			WeakRef<Pipeline> m_Pipeline;
			VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

			VkShaderStageFlags m_Stages = 0;

			std::unordered_map<uint32_t, VkPushConstantRange> m_PushConstantRanges;
			std::unordered_map<std::string, std::pair<VkShaderStageFlags, uint32_t>> m_MemberOffsets;

			std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;
			VkDescriptorSet m_DescriptorSet;
	};
}