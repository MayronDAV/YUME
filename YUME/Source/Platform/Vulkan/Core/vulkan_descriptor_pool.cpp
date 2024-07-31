#include "YUME/yumepch.h"
#include "vulkan_descriptor_pool.h"
#include "vulkan_device.h"



namespace YUME
{
	void VulkanDescriptorPool::Init(uint32_t p_MaxSets, VkDescriptorPoolCreateFlags p_Flags)
	{
		std::array<VkDescriptorPoolSize, 11> poolSizes = {
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLER, p_MaxSets / 2 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, p_MaxSets * 4 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, p_MaxSets },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, p_MaxSets },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, p_MaxSets },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, p_MaxSets },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, p_MaxSets * 2 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, p_MaxSets * 2 },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, p_MaxSets },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, p_MaxSets },
			VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, p_MaxSets / 2 }
		};

		// Create info
		VkDescriptorPoolCreateInfo poolCreateInfo = {};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCreateInfo.flags = p_Flags;
		poolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolCreateInfo.pPoolSizes = poolSizes.data();
		poolCreateInfo.maxSets = p_MaxSets;

		vkCreateDescriptorPool(VulkanDevice::Get().GetDevice(), &poolCreateInfo, VK_NULL_HANDLE, &m_Handle);
	}

	void VulkanDescriptorPool::Reset()
	{
		vkResetDescriptorPool(VulkanDevice::Get().GetDevice(), m_Handle, 0);
	}

	void VulkanDescriptorPool::Destroy()
	{
		YM_CORE_TRACE("Destroying vulkan descriptor pool...")
		vkDestroyDescriptorPool(VulkanDevice::Get().GetDevice(), m_Handle, VK_NULL_HANDLE);
	}
}
