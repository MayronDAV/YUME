#include "YUME/yumepch.h"
#include "vulkan_commandpool.h"

#include "vulkan_device.h"


namespace YUME
{
	VulkanCommandPool::VulkanCommandPool(uint32_t p_QueueIndex, VkCommandPoolCreateFlags p_Flags)
	{
		YM_CORE_TRACE("Creating vulkan command pool...")
		VkCommandPoolCreateInfo cmdPoolCI{};
		cmdPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCI.queueFamilyIndex = p_QueueIndex;
		cmdPoolCI.flags = p_Flags;

		vkCreateCommandPool(VulkanDevice::Get().GetDevice(), &cmdPoolCI, nullptr, &m_Handle);
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		YM_CORE_TRACE("Destroying vulkan command pool...")
		vkDestroyCommandPool(VulkanDevice::Get().GetDevice(), m_Handle, nullptr);
	}

	void VulkanCommandPool::Reset()
	{
		vkResetCommandPool(VulkanDevice::Get().GetDevice(), m_Handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
}