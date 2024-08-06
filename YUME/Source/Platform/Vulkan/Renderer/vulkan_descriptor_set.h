#pragma once
#include "YUME/Renderer/descriptor_set.h"

#include <vulkan/vulkan.h>


namespace YUME
{

	class YM_API VulkanDescriptorSet : public DescriptorSet
	{
		public:
			explicit VulkanDescriptorSet(const Ref<Shader>& p_Shader);
			~VulkanDescriptorSet() override;

			void Bind(uint32_t p_Set = 0) override;
			void Unbind() override;

			void UploadUniform(uint32_t p_Binding, const Ref<UniformBuffer>& p_UniformBuffer) override;
			void UploadTexture2D(uint32_t p_Binding, const Ref<Texture2D>& p_Texture) override;

		private:
			void CheckIfDescriptorSetIsUpdated();

		private:
			std::unordered_map<uint32_t, VkDescriptorSetLayout> m_DescriptorSetLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSets;	
			VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

			std::vector<bool> m_DescriptorUpdated;
			std::vector<bool> m_UsingCurrentPool;

			int m_CurrentBindSet = -1;

	};
}