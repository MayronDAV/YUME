#include "YUME/yumepch.h"
#include "vulkan_commandpool.h"

#include "vulkan_device.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"



namespace YUME
{
	VulkanCommandPool::VulkanCommandPool(uint32_t p_QueueIndex, VkCommandPoolCreateFlags p_Flags, const std::string& p_DebugName)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Creating command pool...")
		VkCommandPoolCreateInfo cmdPoolCI{};
		cmdPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCI.queueFamilyIndex = p_QueueIndex;
		cmdPoolCI.flags = p_Flags;

		vkCreateCommandPool(VulkanDevice::Get().GetDevice(), &cmdPoolCI, nullptr, &m_Handle);

		if (!p_DebugName.empty())
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_COMMAND_POOL, p_DebugName.c_str(), m_Handle);
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying command pool...")
		vkDestroyCommandPool(VulkanDevice::Get().GetDevice(), m_Handle, nullptr);
	}

	void VulkanCommandPool::Reset()
	{
		YM_PROFILE_FUNCTION()

		vkResetCommandPool(VulkanDevice::Get().GetDevice(), m_Handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
}