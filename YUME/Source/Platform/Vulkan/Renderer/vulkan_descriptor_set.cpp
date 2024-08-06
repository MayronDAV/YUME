#include "YUME/yumepch.h"
#include "vulkan_descriptor_set.h"

#include "vulkan_context.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "YUME/Core/application.h"
#include "vulkan_shader.h"
#include "vulkan_uniform_buffer.h"
#include "vulkan_texture.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "Platform/Vulkan/Core/vulkan_descriptor_pool.h"



namespace YUME
{
	struct DescData
	{
		VkDescriptorPool CurrentPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorPool> UsedDescriptorPools;
		std::vector<VkDescriptorPool> FreeDescriptorPools;

		DescData() = default;

		VkDescriptorPool GetPool()
		{
			if (!FreeDescriptorPools.empty())
			{
				VkDescriptorPool pool = FreeDescriptorPools.back();
				FreeDescriptorPools.pop_back();
				return pool;
			}
			else
			{
				VulkanDescriptorPool pool;
				pool.Init(1000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
				return pool.Get();
			}
		}

		void ResetUsedDescriptorPools()
		{
			for (auto& pool : UsedDescriptorPools)
			{
				vkResetDescriptorPool(VulkanDevice::Get().GetDevice(), pool, 0);
				FreeDescriptorPools.push_back(pool);
			}
			UsedDescriptorPools.clear();
		}
	};

	static DescData* s_Data = nullptr;


	VulkanDescriptorSet::VulkanDescriptorSet(const Ref<Shader>& p_Shader)
	{
		if (s_Data == nullptr) s_Data = new DescData();
		if (s_Data->CurrentPool == VK_NULL_HANDLE)
		{
			s_Data->CurrentPool = s_Data->GetPool();
		}

		auto vkShader = static_cast<VulkanShader*>(p_Shader.get());
		m_PipelineLayout = vkShader->GetLayout();
		m_DescriptorSetLayouts = vkShader->GetDescriptorSetLayouts();

		VkDevice device = VulkanDevice::Get().GetDevice();

		m_DescriptorSets.resize(m_DescriptorSetLayouts.size());
		m_DescriptorUpdated.resize(m_DescriptorSetLayouts.size(), false);
		m_UsingCurrentPool.resize(m_DescriptorSets.size(), true);

		for (size_t i = 0; i < m_DescriptorSetLayouts.size(); i++)
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = s_Data->CurrentPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &m_DescriptorSetLayouts[(uint32_t)i];

			if (vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSets[i]) != VK_SUCCESS)
			{
				YM_CORE_ERROR("Failed to allocate Vulkan descriptor sets!")
			}
		}

	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		if (s_Data != nullptr)
		{
			auto usedDescriptorPools = s_Data->UsedDescriptorPools;
			auto freeDescriptorPools = s_Data->FreeDescriptorPools;
			auto currentPool = s_Data->CurrentPool;
			VulkanContext::PushFunction([usedDescriptorPools, freeDescriptorPools, currentPool]()
			{
				VkDevice device = VulkanDevice::Get().GetDevice();

				if (!usedDescriptorPools.empty())
				{
					for (auto pool : usedDescriptorPools)
					{
						vkDestroyDescriptorPool(device, pool, VK_NULL_HANDLE);
					}
				}

				if (!freeDescriptorPools.empty())
				{
					for (auto pool : freeDescriptorPools)
					{
						vkDestroyDescriptorPool(device, pool, VK_NULL_HANDLE);
					}
				}

				if (currentPool != VK_NULL_HANDLE)
				{
					vkDestroyDescriptorPool(device, currentPool, VK_NULL_HANDLE);
				}
			});

			s_Data = nullptr;
		}
	}

	void VulkanDescriptorSet::Bind(uint32_t p_Set)
	{
		m_CurrentBindSet = p_Set;
		CheckIfDescriptorSetIsUpdated();

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			p_Set,
			1,
			&m_DescriptorSets[p_Set],
			0,
			nullptr
		);
	}

	void VulkanDescriptorSet::Unbind()
	{
		m_CurrentBindSet = -1;
		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			0,
			0,
			nullptr,
			0,
			nullptr
		);
	}

	void VulkanDescriptorSet::UploadUniform(uint32_t p_Binding, const Ref<UniformBuffer>& p_UniformBuffer)
	{
		YM_CORE_VERIFY(m_CurrentBindSet >= 0, "Did you call Bind(uint32_t p_Set)?")
		YM_CORE_VERIFY(!m_DescriptorSetLayouts.empty())
		YM_CORE_VERIFY(!m_DescriptorSets.empty())

		auto buffer = static_cast<VulkanUniformBuffer*>(p_UniformBuffer.get());

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = buffer->GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[m_CurrentBindSet];
		descriptorWrite.dstBinding = p_Binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(VulkanDevice::Get().GetDevice(), 1, &descriptorWrite, 0, nullptr);

		m_DescriptorUpdated[m_CurrentBindSet] = true;
	}

	void VulkanDescriptorSet::UploadTexture2D(uint32_t p_Binding, const Ref<Texture2D>& p_Texture)
	{
		YM_CORE_VERIFY(m_CurrentBindSet >= 0, "Did you call Bind(uint32_t p_Set)?")
		YM_CORE_VERIFY(!m_DescriptorSetLayouts.empty())
		YM_CORE_VERIFY(!m_DescriptorSets.empty())

		auto texture = static_cast<VulkanTexture2D*>(p_Texture.get());

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture->GetImageView();
		imageInfo.sampler = texture->GetImageSampler();

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[m_CurrentBindSet];
		descriptorWrite.dstBinding = p_Binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(VulkanDevice::Get().GetDevice(), 1, &descriptorWrite, 0, nullptr);

		m_DescriptorUpdated[m_CurrentBindSet] = true;
	}

	void VulkanDescriptorSet::CheckIfDescriptorSetIsUpdated()
	{
		if (m_CurrentBindSet >= 0 && m_DescriptorUpdated[m_CurrentBindSet])
		{
			m_DescriptorUpdated[m_CurrentBindSet] = false;
			auto device = VulkanDevice::Get().GetDevice();

			if (s_Data->CurrentPool == VK_NULL_HANDLE)
			{
				s_Data->CurrentPool = s_Data->GetPool();
			}

			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.pSetLayouts = &m_DescriptorSetLayouts[m_CurrentBindSet];
			allocInfo.descriptorPool = s_Data->CurrentPool;
			allocInfo.descriptorSetCount = 1;

			auto res = vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSets[m_CurrentBindSet]);

			bool needReallocate = false;
			switch (res)
			{
				case VK_SUCCESS:
					return;
				case VK_ERROR_FRAGMENTED_POOL:
				case VK_ERROR_OUT_OF_POOL_MEMORY:
					needReallocate = true;
					break;
				default:
					YM_CORE_ERROR("Failed to allocate Vulkan descriptor sets!")
					YM_CORE_ERROR("Type: Unrecoverable")
					break;
			}

			if (needReallocate)
			{
				vkDeviceWaitIdle(device);

				s_Data->UsedDescriptorPools.push_back(s_Data->CurrentPool);
				s_Data->CurrentPool = s_Data->GetPool();
				allocInfo.descriptorPool = s_Data->CurrentPool;

				for (int i = 0; i < m_DescriptorSets.size(); i++)
				{
					m_UsingCurrentPool[i] = false;
				}

				if (vkAllocateDescriptorSets(VulkanDevice::Get().GetDevice(), &allocInfo, &m_DescriptorSets[m_CurrentBindSet]) != VK_SUCCESS)
				{
					YM_CORE_ERROR("Failed to allocate Vulkan descriptor sets!")
				}

				m_UsingCurrentPool[m_CurrentBindSet] = true;

				auto& data = s_Data;
				VulkanContext::PushFunctionToFrameEnd([data]() {
					data->ResetUsedDescriptorPools();
				});
			}

		}
	}


}