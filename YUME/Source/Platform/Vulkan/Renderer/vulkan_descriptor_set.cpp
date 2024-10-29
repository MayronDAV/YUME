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
			YM_PROFILE_FUNCTION()

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
			YM_PROFILE_FUNCTION()

			for (auto& pool : UsedDescriptorPools)
			{
				vkResetDescriptorPool(VulkanDevice::Get().GetDevice(), pool, 0);
				FreeDescriptorPools.push_back(pool);
			}
			UsedDescriptorPools.clear();
		}
	};

	static DescData* s_Data = nullptr;


	VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSpec& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		if (s_Data == nullptr) s_Data = new DescData();
		if (s_Data->CurrentPool == VK_NULL_HANDLE)
		{
			s_Data->CurrentPool = s_Data->GetPool();
		}

		m_Set = p_Spec.Set;

		auto vkShader = p_Spec.Shader.As<VulkanShader>();
		m_PipelineLayout = vkShader->GetLayout();
		m_DescriptorsInfo = vkShader->GetDescriptorsInfo(m_Set);
		m_SetLayout = vkShader->GetDescriptorSetLayout(m_Set);

		VkDevice device = VulkanDevice::Get().GetDevice();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = s_Data->CurrentPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_SetLayout;

		auto res = vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet);
		if ( res != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to allocate Vulkan descriptor sets!")
		}

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

			if (vkAllocateDescriptorSets(VulkanDevice::Get().GetDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS)
			{
				YM_CORE_ERROR("Failed to allocate Vulkan descriptor sets!")
			}

			auto& data = s_Data;
			VulkanContext::PushFunctionToFrameEnd([data]()
			{
				data->ResetUsedDescriptorPools();
			});
		}
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		YM_PROFILE_FUNCTION()

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

	void VulkanDescriptorSet::SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				if (descriptor.Buffer)
				{
					descriptor.Buffer.reset();
				}

				descriptor.Buffer = p_UniformBuffer;
				descriptor.Offset = 0;
				descriptor.Size = VK_WHOLE_SIZE;
				m_Queue.push_back((int)i);
				m_MustToBeUploaded = true;
				return;
			}
		}

		YM_CORE_ERROR("Unkown name {}", p_Name)
	}

	void VulkanDescriptorSet::SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_BufferName && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				for (const auto& member : descriptor.Members)
				{
					if (member.Name == p_MemberName)
					{
						if (descriptor.Buffer)
						{
							descriptor.Buffer.reset();
						}

						descriptor.Buffer = UniformBuffer::Create(p_Data, member.Size);
						descriptor.Offset = member.Offset;
						descriptor.Size = member.Size;
						m_Queue.push_back((int)i);
						m_MustToBeUploaded = true;
						return;
					}
				}
			}
		}

		YM_CORE_ERROR("Unkown buffer name {} or member name {}", p_BufferName, p_MemberName)
	}

	void VulkanDescriptorSet::SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, uint32_t p_Size)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_BufferName && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				for (const auto& member : descriptor.Members)
				{
					if (member.Name == p_MemberName)
					{
						if (descriptor.Buffer)
						{
							descriptor.Buffer.reset();
						}

						descriptor.Buffer = UniformBuffer::Create(p_Data, p_Size);
						descriptor.Offset = member.Offset;
						descriptor.Size = p_Size;
						m_Queue.push_back((int)i);
						m_MustToBeUploaded = true;
						return;
					}
				}
			}
		}

		YM_CORE_ERROR("Unkown buffer name {} or member name {}", p_BufferName, p_MemberName)
	}

	void VulkanDescriptorSet::SetTexture2D(const std::string& p_Name, const Ref<Texture2D>& p_Texture)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::IMAGE_SAMPLER)
			{
				descriptor.Textures.clear();
				descriptor.Textures.push_back(p_Texture);
				m_Queue.push_back((int)i);
				m_MustToBeUploaded = true;
				return;
			}
		}

		YM_CORE_ERROR("Unkown name {}", p_Name)
	}

	void VulkanDescriptorSet::SetTexture2D(const std::string& p_Name, const Ref<Texture2D>* p_TextureData, uint32_t p_Count)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::IMAGE_SAMPLER)
			{
				descriptor.Textures.clear();
				descriptor.Textures.resize(p_Count);
				for (uint32_t j = 0; j < p_Count; j++)
				{
					descriptor.Textures[j] = p_TextureData[j];
				}
				m_Queue.push_back((int)i);
				m_MustToBeUploaded = true;
				return;
			}
		}

		YM_CORE_ERROR("Unkown name {}", p_Name)
	}

	void VulkanDescriptorSet::Upload()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(m_SetLayout)
		YM_CORE_VERIFY(m_DescriptorSet)

		std::vector<VkWriteDescriptorSet> descWrites;
		std::vector<VkDescriptorBufferInfo> buffersInfo;
		std::unordered_map<int, std::vector<VkDescriptorImageInfo>> imagesInfoMap;

		for (int index : m_Queue)
		{
			auto& data = m_DescriptorsInfo[index];

			VkWriteDescriptorSet& descriptorWrite = descWrites.emplace_back();

			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSet;
			descriptorWrite.dstBinding = data.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = Utils::DescriptorTypeToVk(data.Type);
			descriptorWrite.descriptorCount = 1;

			auto& bufferInfo = buffersInfo.emplace_back();
			auto& imagesInfo = imagesInfoMap[index];

			if (data.Type == DescriptorType::IMAGE_SAMPLER)
			{
				for (const auto& texture : data.Textures)
				{
					TransitionImageToCorrectLayout(texture);
					auto vkTexture = texture.As<VulkanTexture2D>();

					auto& info = imagesInfo.emplace_back();
					info.imageLayout = vkTexture->GetLayout();
					info.imageView = vkTexture->GetImageView();
					info.sampler = vkTexture->GetImageSampler();
				}

				descriptorWrite.descriptorCount = data.Size;
				descriptorWrite.pImageInfo = imagesInfo.data();
			}
			else if (data.Type == DescriptorType::UNIFORM_BUFFER)
			{
				bufferInfo.buffer = data.Buffer.As<VulkanUniformBuffer>()->GetBuffer();
				bufferInfo.offset = data.Offset;
				bufferInfo.range = data.Size;

				descriptorWrite.pBufferInfo = &bufferInfo;
			}
		}

		vkUpdateDescriptorSets(VulkanDevice::Get().GetDevice(), (uint32_t)descWrites.size(), descWrites.data(), 0, nullptr);

		m_Queue.clear();
		m_MustToBeUploaded = false;
	}

	void VulkanDescriptorSet::Bind()
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(!m_MustToBeUploaded, "Did you call Upload()?")

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			m_Set,
			1,
			&m_DescriptorSet,
			0,
			nullptr
		);
	}

	void VulkanDescriptorSet::TransitionImageToCorrectLayout(const Ref<Texture2D>& p_Texture)
	{
		if (!p_Texture)
			return;

		const auto& spec = p_Texture->GetSpecification();
		const auto& vkTexture = p_Texture.As<VulkanTexture2D>();
		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED)
		{
			if (vkTexture->GetLayout() != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, false);
			}
		}
		else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
		{
			if (vkTexture->GetLayout() != VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, false);
			}
		}
	}

}