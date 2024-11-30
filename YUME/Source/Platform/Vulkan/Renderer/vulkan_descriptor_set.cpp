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
#include "vulkan_storage_buffer.h"



namespace YUME
{
	VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSpec& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		m_Set = p_Spec.Set;

		auto vkShader = p_Spec.Shader.As<VulkanShader>();
		m_PipelineLayout = vkShader->GetLayout();
		m_DescriptorsInfo = vkShader->GetDescriptorsInfo(m_Set);
		m_SetLayout = vkShader->GetDescriptorSetLayout(m_Set);

		VkDevice device = VulkanDevice::Get().GetDevice();

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = VulkanDevice::Get().GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_SetLayout;

		auto res = vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet);
		if ( res != VK_SUCCESS)
		{
			YM_CORE_ERROR(VULKAN_PREFIX "Failed to allocate descriptor sets!")
		}
	}

	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		YM_PROFILE_FUNCTION()
	}

	void VulkanDescriptorSet::SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::UNIFORM_BUFFER)
			{
				if (descriptor.UBuffer)
				{
					descriptor.UBuffer.reset();
				}

				descriptor.UBuffer = p_UniformBuffer;
				descriptor.Offset = 0;
				descriptor.Size = VK_WHOLE_SIZE;
				m_Queue.push_back((int)i);
				m_MustToBeUploaded = true;
				return;
			}
		}

		YM_CORE_ERROR(VULKAN_PREFIX "Unkown name {}", p_Name)
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
						if (descriptor.UBuffer)
						{
							descriptor.UBuffer.reset();
						}

						descriptor.UBuffer = UniformBuffer::Create(p_Data, (uint32_t)member.Size);
						descriptor.Offset = member.Offset;
						descriptor.Size = member.Size;
						m_Queue.push_back((int)i);
						m_MustToBeUploaded = true;
						return;
					}
				}
			}
		}

		YM_CORE_ERROR(VULKAN_PREFIX "Unkown buffer name {} or member name {}", p_BufferName, p_MemberName)
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
						if (descriptor.UBuffer)
						{
							descriptor.UBuffer.reset();
						}

						descriptor.UBuffer = UniformBuffer::Create(p_Data, p_Size);
						descriptor.Offset = member.Offset;
						descriptor.Size = p_Size;
						m_Queue.push_back((int)i);
						m_MustToBeUploaded = true;
						return;
					}
				}
			}
		}

		YM_CORE_ERROR(VULKAN_PREFIX "Unkown buffer name {} or member name {}", p_BufferName, p_MemberName)
	}

	void VulkanDescriptorSet::SetStorageData(const std::string& p_Name, const Ref<StorageBuffer>& p_StorageBuffer)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if (descriptor.Name == p_Name && descriptor.Type == DescriptorType::STORAGE_BUFFER)
			{
				if (descriptor.SBuffer)
				{
					descriptor.SBuffer.reset();
				}

				descriptor.SBuffer = p_StorageBuffer;
				descriptor.Offset = 0;
				descriptor.Size = VK_WHOLE_SIZE;
				m_Queue.push_back((int)i);
				m_MustToBeUploaded = true;
				return;
			}
		}

		YM_CORE_ERROR(VULKAN_PREFIX "Unkown name {}", p_Name)
	}

	void VulkanDescriptorSet::SetTexture(const std::string& p_Name, const Ref<Texture>& p_Texture)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if ( descriptor.Name == p_Name && 
				(descriptor.Type == DescriptorType::IMAGE_SAMPLER || 
				 descriptor.Type == DescriptorType::STORAGE_IMAGE))
			{
				descriptor.Textures.clear();
				descriptor.Textures.push_back(p_Texture);
				m_Queue.push_back((int)i);
				m_MustToBeUploaded = true;
				return;
			}
		}

		YM_CORE_ERROR(VULKAN_PREFIX "Unkown name {}", p_Name)
	}


	void VulkanDescriptorSet::SetTexture(const std::string& p_Name, const Ref<Texture>* p_TextureData, uint32_t p_Count)
	{
		YM_PROFILE_FUNCTION()

		for (size_t i = 0; i < m_DescriptorsInfo.size(); i++)
		{
			auto& descriptor = m_DescriptorsInfo[i];
			if ( descriptor.Name == p_Name &&
				(descriptor.Type == DescriptorType::IMAGE_SAMPLER ||
				 descriptor.Type == DescriptorType::STORAGE_IMAGE))
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

		YM_CORE_ERROR(VULKAN_PREFIX "Unkown name {}", p_Name)
	}

	void VulkanDescriptorSet::Upload(CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_ASSERT(m_SetLayout)
		YM_CORE_ASSERT(m_DescriptorSet)

		std::vector<VkWriteDescriptorSet> descWrites;
		std::vector<VkDescriptorBufferInfo> buffersInfo;
		std::unordered_map<int, std::vector<VkDescriptorImageInfo>> imagesInfoMap;

		for (int index : m_Queue)
		{
			auto& data = m_DescriptorsInfo[index];

			VkWriteDescriptorSet descriptorWrite{};

			descriptorWrite.sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet			= m_DescriptorSet;
			descriptorWrite.dstBinding		= data.Binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType  = VKUtils::DescriptorTypeToVk(data.Type);
			descriptorWrite.descriptorCount = 1;

			auto& bufferInfo = buffersInfo.emplace_back();
			auto& imagesInfo = imagesInfoMap[index];

			if (data.Type == DescriptorType::IMAGE_SAMPLER)
			{
				for (const auto& texture : data.Textures)
				{
					if (!texture)
					{
						YM_CORE_ERROR(VULKAN_PREFIX "Texture is nullptr")
						continue;
					}

					TransitionImageToCorrectLayout(texture, p_CommandBuffer);

					VkImageLayout layout	  = VK_IMAGE_LAYOUT_UNDEFINED;
					VkImageView	  view		  = VK_NULL_HANDLE;
					VkSampler	  sampler	  = VK_NULL_HANDLE;

					if (texture->GetType() == AssetType::Texture2D)
					{
						const auto& vkTexture = texture.As<VulkanTexture2D>();
						layout				  = vkTexture->GetLayout();
						view				  = vkTexture->GetImageView();
						sampler				  = vkTexture->GetImageSampler();
					}
					else if (texture->GetType() == AssetType::TextureArray)
					{
						const auto& vkTexture = texture.As<VulkanTextureArray>();
						layout				  = vkTexture->GetLayout();
						view				  = vkTexture->GetImageView();
						sampler				  = vkTexture->GetImageSampler();
					}
					else
					{
						YM_CORE_ERROR(VULKAN_PREFIX "Unknown texture type - Texture: {}!", texture->GetSpecification().DebugName)
						continue;
					}

					auto& info				  = imagesInfo.emplace_back();
					info.imageLayout		  = layout;
					info.imageView			  = view;
					info.sampler			  = sampler;
				}

				descriptorWrite.descriptorCount = (uint32_t)data.Size;
				descriptorWrite.pImageInfo		= imagesInfo.data();
			}
			else if (data.Type == DescriptorType::STORAGE_IMAGE)
			{
				for (const auto& texture : data.Textures)
				{
					auto vkTexture = texture.As<VulkanTexture2D>();
					TransitionImageToCorrectLayout(vkTexture, p_CommandBuffer);

					auto& info		 = imagesInfo.emplace_back();
					info.imageLayout = vkTexture->GetLayout();
					info.imageView	 = vkTexture->GetImageView();
				}

				descriptorWrite.descriptorCount = (uint32_t)data.Size;
				descriptorWrite.pImageInfo		= imagesInfo.data();
			}
			else if (data.Type == DescriptorType::UNIFORM_BUFFER)
			{
				bufferInfo.buffer = data.UBuffer.As<VulkanUniformBuffer>()->GetBuffer();
				bufferInfo.offset = data.Offset;
				bufferInfo.range  = data.Size;

				descriptorWrite.pBufferInfo = &bufferInfo;
			}
			else if (data.Type == DescriptorType::STORAGE_BUFFER)
			{
				bufferInfo.buffer = data.SBuffer.As<VulkanStorageBuffer>()->GetBuffer();
				bufferInfo.offset = data.Offset;
				bufferInfo.range  = data.Size;

				descriptorWrite.pBufferInfo = &bufferInfo;
			}
			else
			{
				YM_CORE_ERROR(VULKAN_PREFIX "Unsupported descriptor type {} !", (int)data.Type)
				continue;
			}

			descWrites.push_back(descriptorWrite);
		}

		if (descWrites.empty())
		{
			YM_CORE_ERROR(VULKAN_PREFIX "You called upload before sending data")
			m_Queue.clear();
			m_MustToBeUploaded = false;
			return;
		}

		vkUpdateDescriptorSets(VulkanDevice::Get().GetDevice(), (uint32_t)descWrites.size(), descWrites.data(), 0, nullptr);

		m_Queue.clear();
		m_MustToBeUploaded = false;
	}

	void VulkanDescriptorSet::Bind(CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(!m_MustToBeUploaded, "Did you call Upload()?")

		auto& commandBuffer = static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle();

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

	void VulkanDescriptorSet::TransitionImageToCorrectLayout(const Ref<Texture>& p_Texture, CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		if (!p_Texture)
			return;

		const auto& spec = p_Texture->GetSpecification();
		auto cmd = p_CommandBuffer ? p_CommandBuffer : VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer.get();

		if (p_Texture->GetType() == AssetType::Texture2D)
		{
			const auto& vkTexture = p_Texture.As<VulkanTexture2D>();
			if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd);
			}
			else if (spec.Usage == TextureUsage::TEXTURE_STORAGE)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_GENERAL, cmd);
			}
			else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, cmd);
			}
		}
		else if (p_Texture->GetType() == AssetType::TextureArray)
		{
			const auto& vkTexture = p_Texture.As<VulkanTextureArray>();
			if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, cmd);
			}
			else if (spec.Usage == TextureUsage::TEXTURE_STORAGE)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_GENERAL, cmd);
			}
			else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
			{
				vkTexture->TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, cmd);
			}
		}
	}

}