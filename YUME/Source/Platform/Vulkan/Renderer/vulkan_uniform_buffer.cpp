#include "YUME/yumepch.h"
#include "vulkan_uniform_buffer.h"

#include "vulkan_context.h"
#include "Platform/Vulkan/Core/vulkan_device.h"




namespace YUME
{
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t p_SizeBytes, uint32_t p_Offset, uint32_t p_Binding)
		: m_Binding(p_Binding), m_Offset(p_Offset)
	{
		auto device = VulkanDevice::Get().GetDevice();

		m_SizeBytes = p_SizeBytes;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = p_SizeBytes;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, VK_NULL_HANDLE, &m_Buffer) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to create uniform buffer!")
			return;
		}

		vkGetBufferMemoryRequirements(device, m_Buffer, &m_Requirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = m_Requirements.size;
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(m_Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		if (vkAllocateMemory(device, &allocInfo, VK_NULL_HANDLE, &m_Memory) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to allocate uniform buffer memory!")
			return;
		}
	}

	VulkanUniformBuffer::VulkanUniformBuffer(const void* p_Data, uint32_t p_SizeBytes, uint32_t p_Offset, uint32_t p_Binding)
		: m_Binding(p_Binding), m_Offset(p_Offset)
	{
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		auto buffer = m_Buffer;
		auto memory = m_Memory;
		VulkanContext::PushFunction([buffer, memory]()
		{
			auto device = VulkanDevice::Get().GetDevice();

			YM_CORE_TRACE("Destroying vulkan vertex buffer")

			if (buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device, buffer, VK_NULL_HANDLE);
			if (memory != VK_NULL_HANDLE)
				vkFreeMemory(device, memory, VK_NULL_HANDLE);
		});
	}

	void VulkanUniformBuffer::SetData(const void* p_Data, uint32_t p_SizeBytes)
	{
	}
}
