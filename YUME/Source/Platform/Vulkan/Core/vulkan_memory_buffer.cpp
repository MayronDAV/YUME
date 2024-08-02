#include "YUME/yumepch.h"
#include "vulkan_memory_buffer.h"
#include "vulkan_device.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"

#include "YUME/Core/application.h"



namespace YUME
{
	VulkanMemoryBuffer::VulkanMemoryBuffer(VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_MemoryProperyFlags, VkDeviceSize p_SizeBytes)
	{
		Init(p_Usage, p_MemoryProperyFlags, p_SizeBytes);
	}

	VulkanMemoryBuffer::~VulkanMemoryBuffer()
	{
		Destroy(!m_DeleteWithoutQueue);
	}

	void VulkanMemoryBuffer::Init(VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_MemoryPropertyFlags, VkDeviceSize p_SizeBytes)
	{
		YM_CORE_VERIFY(p_SizeBytes > 0)

		auto device = VulkanDevice::Get().GetDevice();
		m_UsageFlags = p_Usage;
		m_MemoryPropertyFlags = p_MemoryPropertyFlags;
		m_SizeBytes = p_SizeBytes;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = p_SizeBytes;
		bufferInfo.usage = p_Usage;
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
		allocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(m_Requirements.memoryTypeBits, m_MemoryPropertyFlags);

		if (vkAllocateMemory(device, &allocInfo, VK_NULL_HANDLE, &m_Memory) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to allocate index buffer memory!")
			return;
		}

		vkBindBufferMemory(device, m_Buffer, m_Memory, 0);
	}

	void VulkanMemoryBuffer::Resize(VkDeviceSize p_SizeBytes, const void* p_Data)
	{
		Destroy(!m_DeleteWithoutQueue);
		Init(m_UsageFlags, m_MemoryPropertyFlags, p_SizeBytes);
	}

	void VulkanMemoryBuffer::SetData(VkDeviceSize p_SizeBytes, const void* p_Data, VkDeviceSize p_Offset, bool p_AddBarrier)
	{
		YM_CORE_VERIFY(p_Data != nullptr && p_SizeBytes > 0)

		auto device = VulkanDevice::Get().GetDevice();

		if (m_MemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			Map(p_SizeBytes, p_Offset);
			memcpy(m_Mapped, p_Data, p_SizeBytes);
			UnMap();
		}
		else
		{
			VkBufferCreateInfo stagingBufferInfo = {};
			stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			stagingBufferInfo.size = p_SizeBytes;
			stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkBuffer stagingBuffer;
			if (vkCreateBuffer(device, &stagingBufferInfo, VK_NULL_HANDLE, &stagingBuffer) != VK_SUCCESS)
			{
				YM_CORE_ERROR("Failed to create vertex buffer!")
				return;
			}

			VkMemoryRequirements stagingMemRequirements;
			vkGetBufferMemoryRequirements(device, stagingBuffer, &stagingMemRequirements);

			VkMemoryAllocateInfo stagingAllocInfo = {};
			stagingAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			stagingAllocInfo.allocationSize = stagingMemRequirements.size;
			stagingAllocInfo.memoryTypeIndex = VulkanDevice::Get().FindMemoryType(stagingMemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			VkDeviceMemory stagingBufferMemory;
			if (vkAllocateMemory(device, &stagingAllocInfo, VK_NULL_HANDLE, &stagingBufferMemory) != VK_SUCCESS) {
				YM_CORE_ERROR("Failed to allocate staging buffer memory!")
				vkDestroyBuffer(device, stagingBuffer, VK_NULL_HANDLE);
				return;
			}

			vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

			void* data;
			if (vkMapMemory(device, stagingBufferMemory, p_Offset, p_SizeBytes, 0, &data) != VK_SUCCESS)
			{
				vkDestroyBuffer(device, stagingBuffer, VK_NULL_HANDLE);
				vkFreeMemory(device, stagingBufferMemory, VK_NULL_HANDLE);
			}
			memcpy(data, p_Data, p_SizeBytes);
			vkUnmapMemory(device, stagingBufferMemory);

			//vkBindBufferMemory(device, m_Buffer, m_Memory, 0);

			auto commandBuffer = Utils::BeginSingleTimeCommand();

			VkBufferCopy copyRegion = {};
			copyRegion.size = p_SizeBytes;
			vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_Buffer, 1, &copyRegion);

			Utils::EndSingleTimeCommand(commandBuffer);

			vkDestroyBuffer(device, stagingBuffer, VK_NULL_HANDLE);
			vkFreeMemory(device, stagingBufferMemory, VK_NULL_HANDLE);
		}

		if (p_AddBarrier)
		{
			auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
			auto commandBuffer = context->GetCommandBuffer();

			VkBufferMemoryBarrier bufferBarrier = {};
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			bufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			bufferBarrier.buffer = m_Buffer;
			bufferBarrier.offset = 0;
			bufferBarrier.size = VK_WHOLE_SIZE;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
				0, 0,
				nullptr,
				1,
				&bufferBarrier,
				0,
				nullptr
			);
		}
	}

	void VulkanMemoryBuffer::Map(VkDeviceSize p_SizeBytes, VkDeviceSize p_Offset)
	{
		YM_CORE_VERIFY(p_SizeBytes > 0)

		if (vkMapMemory(VulkanDevice::Get().GetDevice(), m_Memory, p_Offset, p_SizeBytes, 0, &m_Mapped) != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to map memory!")
			Destroy(false);
		}
	}

	void VulkanMemoryBuffer::UnMap()
	{
		if (m_Mapped)
		{
			vkUnmapMemory(VulkanDevice::Get().GetDevice(), m_Memory);
			m_Mapped = nullptr;
			return;
		}

		YM_CORE_ERROR("Did you call Map()?")
	}

	void VulkanMemoryBuffer::Flush(VkDeviceSize p_SizeBytes, VkDeviceSize p_Offset)
	{
		YM_CORE_VERIFY(p_SizeBytes > 0)

		VkMappedMemoryRange mappedRange = {};
		mappedRange.memory = m_Memory;
		mappedRange.offset = p_Offset;
		mappedRange.size = p_SizeBytes;
		vkFlushMappedMemoryRanges(VulkanDevice::Get().GetDevice(), 1, &mappedRange);
	}

	void VulkanMemoryBuffer::Invalidate(VkDeviceSize p_SizeBytes, VkDeviceSize p_Offset)
	{
		YM_CORE_VERIFY(p_SizeBytes > 0)

		VkMappedMemoryRange mappedRange = {};
		mappedRange.memory = m_Memory;
		mappedRange.offset = p_Offset;
		mappedRange.size = p_SizeBytes;
		vkInvalidateMappedMemoryRanges(VulkanDevice::Get().GetDevice(), 1, &mappedRange);
	}

	void VulkanMemoryBuffer::Destroy(bool p_DeletionQueue) noexcept
	{
		auto device = VulkanDevice::Get().GetDevice();
		auto buffer = m_Buffer;
		auto memory = m_Memory;

		if (p_DeletionQueue)
		{
			VulkanContext::PushFunction([device, buffer, memory]()
			{
				YM_CORE_TRACE("Destroying vulkan vertex buffer")

				if (buffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, buffer, VK_NULL_HANDLE);
				if (memory != VK_NULL_HANDLE)
					vkFreeMemory(device, memory, VK_NULL_HANDLE);
			});
		}
		else
		{
			if (buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device, buffer, VK_NULL_HANDLE);
			if (memory != VK_NULL_HANDLE)
				vkFreeMemory(device, memory, VK_NULL_HANDLE);
		}
	}
}
