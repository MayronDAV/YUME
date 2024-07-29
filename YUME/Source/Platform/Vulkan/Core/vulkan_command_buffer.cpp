#include "YUME/yumepch.h"
#include "vulkan_command_buffer.h"

#include "vulkan_device.h"




namespace YUME
{
	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Free();
	}

	void VulkanCommandBuffer::Free(int p_Index)
	{
		if (!m_CommandBuffers.empty())
		{
			YM_CORE_TRACE("Destroying vulkan command buffer...")
			vkFreeCommandBuffers(VulkanDevice::Get().GetDevice(), VulkanDevice::Get().GetCommandPool(), 1, &m_CommandBuffers[p_Index]);
			m_CommandBuffers.erase(m_CommandBuffers.begin() + p_Index);
		}
	}

	void VulkanCommandBuffer::Free()
	{
		if (!m_CommandBuffers.empty())
		{
			YM_CORE_TRACE("Destroying vulkan command buffers...")
			vkFreeCommandBuffers(VulkanDevice::Get().GetDevice(), VulkanDevice::Get().GetCommandPool(), m_CommandBuffers.size(), m_CommandBuffers.data());
			m_CommandBuffers.clear();
		}
	}

	void VulkanCommandBuffer::Reset() const
	{
		for (auto& buffer : m_CommandBuffers)
		{
			vkResetCommandBuffer(buffer, 0);
		}
		
	}

	void VulkanCommandBuffer::Reset(int p_Index)
	{
		vkResetCommandBuffer(m_CommandBuffers[p_Index], 0);
	}

	void VulkanCommandBuffer::Init(int p_Count)
	{
		m_Count = p_Count;

		auto commandPool = VulkanDevice::Get().GetCommandPool();
		auto& device = VulkanDevice::Get().GetDevice();

		m_CommandBuffers.resize(p_Count);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)p_Count;

		auto res = vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

	void VulkanCommandBuffer::Begin(int p_Index)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		auto res = vkBeginCommandBuffer(m_CommandBuffers[p_Index], &beginInfo);
		YM_CORE_ASSERT(res == VK_SUCCESS)
	}

	void VulkanCommandBuffer::End(int p_Index)
	{
		auto res = vkEndCommandBuffer(m_CommandBuffers[p_Index]);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}
}