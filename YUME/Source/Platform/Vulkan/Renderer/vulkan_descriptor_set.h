#pragma once
#include "YUME/Renderer/descriptor_set.h"

#include <vulkan/vulkan.h>


namespace YUME
{
	class VulkanDescriptorSet : public DescriptorSet
	{
		public:
			explicit VulkanDescriptorSet(const DescriptorSpec& p_Spec);
			~VulkanDescriptorSet() override;

			void SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer) override;
			void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data) override;
			void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, uint32_t p_Size) override;

			void SetTexture2D(const std::string& p_Name, const Ref<Texture2D>& p_Texture) override;
			void SetTexture2D(const std::string& p_Name, const Ref<Texture2D>* p_TextureData, uint32_t p_Count) override;

			void Upload() override;

		protected:
			void Bind() override;

		private:
			void TransitionImageToCorrectLayout(const Ref<Texture2D>& p_Texture);

		private:
			VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

			int m_Set = -1;
			VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
			VkDescriptorSetLayout m_SetLayout = VK_NULL_HANDLE;
			std::vector<DescriptorInfo> m_DescriptorsInfo;
			std::vector<int> m_Queue;

			bool m_MustToBeUploaded = false;
	};
}