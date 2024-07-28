#include "YUME/yumepch.h"
#include "vulkan_buffer.h"

#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_context.h"
#include "YUME/Core/application.h"



namespace YUME
{

	VulkanVertexBuffer::VulkanVertexBuffer(uint64_t p_SizeBytes)
	{
		auto device = VulkanDevice::Get().GetDevice();

		m_SizeBytes = p_SizeBytes;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = p_SizeBytes;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, VK_NULL_HANDLE, &m_Buffer) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to create vertex buffer!")
			return;
		}

		vkGetBufferMemoryRequirements(device, m_Buffer, &m_Requirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = m_Requirements.size;
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(m_Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, VK_NULL_HANDLE, &m_Memory) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to allocate index buffer memory!")
				return;
		}
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const float* p_Vertices, uint64_t p_SizeBytes)
	{
		auto device = VulkanDevice::Get().GetDevice();

		m_SizeBytes = p_SizeBytes;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = p_SizeBytes;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, VK_NULL_HANDLE, &m_Buffer) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to create vertex buffer!")
			return;
		}

		vkGetBufferMemoryRequirements(device, m_Buffer, &m_Requirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = m_Requirements.size;
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(m_Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, VK_NULL_HANDLE, &m_Memory) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to allocate index buffer memory!")
			return;
		}

		vkBindBufferMemory(device, m_Buffer, m_Memory, 0);

		void* data;
		vkMapMemory(device, m_Memory, 0, p_SizeBytes, 0, &data);
		memcpy(data, p_Vertices, p_SizeBytes);
		vkUnmapMemory(device, m_Memory);
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		auto device = VulkanDevice::Get().GetDevice();

		YM_CORE_TRACE("Destroying vulkan vertex buffer")

		if (m_Buffer != VK_NULL_HANDLE)
			vkDestroyBuffer(device, m_Buffer, VK_NULL_HANDLE);
		if (m_Memory != VK_NULL_HANDLE)
			vkFreeMemory(device, m_Memory, VK_NULL_HANDLE);

	}

	void VulkanVertexBuffer::Bind() const
	{
		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto currentFrame = context->GetCurrentFrame();
		auto commandBuffer = context->GetCommandBuffer().Get(currentFrame);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_Buffer, &offset);
	}

	void VulkanVertexBuffer::Unbind() const
	{
		//auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		//auto currentFrame = context->GetCurrentFrame();
		//auto commandBuffer = context->GetCommandBuffer().Get(currentFrame);

		//VkBuffer nullBuffer = VK_NULL_HANDLE;
		//VkDeviceSize nullOffset = 0;
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &nullBuffer, &nullOffset);
	}

	void VulkanVertexBuffer::SetData(const void* p_Data, uint64_t p_SizeBytes)
	{
		auto device = VulkanDevice::Get().GetDevice();

		vkBindBufferMemory(device, m_Buffer, m_Memory, 0);

		void* data;
		vkMapMemory(device, m_Memory, 0, p_SizeBytes, 0, &data);
		memcpy(data, p_Data, p_SizeBytes);
		vkUnmapMemory(device, m_Memory);
	}



	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* p_Indices, uint32_t p_Count)
	{
		auto device = VulkanDevice::Get().GetDevice();

		m_Count = p_Count;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = p_Count * sizeof(uint32_t);
		bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS) 
		{
			YM_CORE_ERROR("Failed to create index buffer!")
			return;
		}

		vkGetBufferMemoryRequirements(device, m_Buffer, &m_Requirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = m_Requirements.size;
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(m_Requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to allocate index buffer memory!")
			return;
		}

		vkBindBufferMemory(device, m_Buffer, m_Memory, 0);

		void* data;
		vkMapMemory(device, m_Memory, 0, bufferInfo.size, 0, &data);
		memcpy(data, p_Indices, bufferInfo.size); // `indices` é o array de índices
		vkUnmapMemory(device, m_Memory);
	}

	VulkanIndexBuffer::~VulkanIndexBuffer()
	{
		auto device = VulkanDevice::Get().GetDevice();

		YM_CORE_TRACE("Destroying vulkan index buffer")

		if (m_Buffer != VK_NULL_HANDLE)
			vkDestroyBuffer(device, m_Buffer, VK_NULL_HANDLE);
		if (m_Memory != VK_NULL_HANDLE)
			vkFreeMemory(device, m_Memory, VK_NULL_HANDLE);
	}

	void VulkanIndexBuffer::Bind() const
	{
		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto currentFrame = context->GetCurrentFrame();
		auto commandBuffer = context->GetCommandBuffer().Get(currentFrame);

		VkDeviceSize offset = 0;
		vkCmdBindIndexBuffer(commandBuffer, m_Buffer, 0, VK_INDEX_TYPE_UINT32);
	}
	void VulkanIndexBuffer::Unbind() const
	{
		// TODO:

		//YM_CORE_VERIFY(false, "Not implemented!")
		//VkBuffer nullBuffer = VK_NULL_HANDLE;
		//vkCmdBindIndexBuffer(nullptr /* command buffer  */, nullBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
}
