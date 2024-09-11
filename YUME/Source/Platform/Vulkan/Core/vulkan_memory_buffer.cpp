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
		YM_PROFILE_FUNCTION()

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

#ifdef USE_VMA_ALLOCATOR
		bool isMappable = (p_MemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;

		VmaAllocationCreateInfo vmaAllocInfo = {};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaAllocInfo.flags = isMappable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : 0;
		vmaAllocInfo.preferredFlags = p_MemoryPropertyFlags;

	#ifdef USE_SMALL_VMA_POOL
		if (bufferInfo.size <= SMALL_ALLOCATION_MAX_SIZE)
		{
			uint32_t mem_type_index = 0;
			vmaFindMemoryTypeIndexForBufferInfo(VulkanDevice::Get().GetAllocator(), &bufferInfo, &vmaAllocInfo, &mem_type_index);
			vmaAllocInfo.pool = VulkanDevice::Get().GetOrCreateSmallAllocPool(mem_type_index);
		}
	#endif

		vmaCreateBuffer(VulkanDevice::Get().GetAllocator(), &bufferInfo, &vmaAllocInfo, &m_Buffer, &m_Allocation, nullptr);
#else
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
#endif
	}

	void VulkanMemoryBuffer::Resize(VkDeviceSize p_SizeBytes, const void* p_Data)
	{
		YM_PROFILE_FUNCTION()

		Destroy(!m_DeleteWithoutQueue);
		Init(m_UsageFlags, m_MemoryPropertyFlags, p_SizeBytes);
	}

	void VulkanMemoryBuffer::SetData(VkDeviceSize p_SizeBytes, const void* p_Data, VkDeviceSize p_Offset)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_VERIFY(p_Data != nullptr && p_SizeBytes > 0)

		if (p_SizeBytes != m_SizeBytes)
		{
			auto buffer = m_Buffer;
			auto device = VulkanDevice::Get().GetDevice();

		#ifdef USE_VMA_ALLOCATOR
			auto alloc = m_Allocation;
			VulkanContext::PushFunctionToFrameEnd([device, buffer, alloc]()
			{
				vkDeviceWaitIdle(device);
				vmaDestroyBuffer(VulkanDevice::Get().GetAllocator(), buffer, alloc);
			});
		#else
			auto memory = m_Memory;
			VulkanContext::PushFunctionToFrameEnd([device, buffer, memory]()
			{
				vkDeviceWaitIdle(device);

				if (buffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, buffer, VK_NULL_HANDLE);
				if (memory != VK_NULL_HANDLE)
					vkFreeMemory(device, memory, VK_NULL_HANDLE);
			});
		#endif

			Init(m_UsageFlags, m_MemoryPropertyFlags, p_SizeBytes);
		}

		if (m_MemoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			Map(p_SizeBytes, p_Offset);
			memcpy(m_Mapped, p_Data, p_SizeBytes);
			UnMap();
		}
		else
		{
#ifdef USE_VMA_ALLOCATOR
			auto allocator = VulkanDevice::Get().GetAllocator();

			VmaAllocationCreateInfo vmaAllocInfo = {};
			vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			vmaAllocInfo.preferredFlags = m_MemoryPropertyFlags;
			vmaAllocInfo.flags = 0;

			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = p_SizeBytes;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VkBuffer stagingBuffer;
			VmaAllocation stagingAlloc;

	#ifdef USE_SMALL_VMA_POOL
			if (bufferCreateInfo.size <= SMALL_ALLOCATION_MAX_SIZE)
			{
				uint32_t mem_type_index = 0;
				vmaFindMemoryTypeIndexForBufferInfo(allocator, &bufferCreateInfo, &vmaAllocInfo, &mem_type_index);
				vmaAllocInfo.pool = VulkanDevice::Get().GetOrCreateSmallAllocPool(mem_type_index);
			}
	#endif

			vmaCreateBuffer(allocator, &bufferCreateInfo, &vmaAllocInfo, &stagingBuffer, &stagingAlloc, nullptr);

			// Copy data to staging buffer
			uint8_t* destData;
			{
				auto res = vmaMapMemory(allocator, stagingAlloc, (void**)&destData);
				if (res != VK_SUCCESS)
				{
					YM_CORE_CRITICAL("Failed to map buffer")
					vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
					return;
				}

				memcpy(destData, p_Data, p_SizeBytes);
				vmaUnmapMemory(allocator, stagingAlloc);
			}

			VkCommandBuffer commandBuffer = Utils::BeginSingleTimeCommand();

			VkBufferCopy copyRegion = {};
			copyRegion.size = p_SizeBytes;
			vkCmdCopyBuffer(
				commandBuffer,
				stagingBuffer,
				m_Buffer,
				1,
				&copyRegion);

			Utils::EndSingleTimeCommand(commandBuffer);
			vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);

#else
			auto device = VulkanDevice::Get().GetDevice();

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
#endif
		}
	}

	void VulkanMemoryBuffer::Map(VkDeviceSize p_SizeBytes, VkDeviceSize p_Offset)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(p_SizeBytes > 0)

	#ifdef USE_VMA_ALLOCATOR
			auto res = vmaMapMemory(VulkanDevice::Get().GetAllocator(), m_Allocation, &m_Mapped);
	#else
			auto res = vkMapMemory(VulkanDevice::Get().GetDevice(), m_Memory, p_Offset, p_SizeBytes, 0, &m_Mapped);
	#endif

		if (res != VK_SUCCESS)
		{
			YM_CORE_ERROR("Failed to map memory!")
			Destroy(false);
		}
	}

	void VulkanMemoryBuffer::UnMap()
	{
		YM_PROFILE_FUNCTION()

		if (m_Mapped)
		{
		#ifdef USE_VMA_ALLOCATOR
			vmaUnmapMemory(VulkanDevice::Get().GetAllocator(), m_Allocation);
		#else
			vkUnmapMemory(VulkanDevice::Device(), m_Memory);
		#endif

			m_Mapped = nullptr;
			return;
		}

		YM_CORE_ERROR("Did you call Map()?")
		Destroy(false);
	}

	void VulkanMemoryBuffer::Flush(VkDeviceSize p_SizeBytes, VkDeviceSize p_Offset)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(p_SizeBytes > 0)

#ifdef USE_VMA_ALLOCATOR
		vmaFlushAllocation(VulkanDevice::Get().GetAllocator(), m_Allocation, p_Offset, p_SizeBytes);
#else
		Map();

		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = m_Memory;
		mappedRange.offset = p_Offset;
		mappedRange.size = p_SizeBytes;
		vkFlushMappedMemoryRanges(VulkanDevice::Get().GetDevice(), 1, &mappedRange);

		UnMap();
#endif
	}

	void VulkanMemoryBuffer::Invalidate(VkDeviceSize p_SizeBytes, VkDeviceSize p_Offset)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(p_SizeBytes > 0)

#ifdef USE_VMA_ALLOCATOR
		vmaInvalidateAllocation(VulkanDevice::Get().GetAllocator(), m_Allocation, p_Offset, p_SizeBytes);
#else
		VkMappedMemoryRange mappedRange = {};
		mappedRange.memory = m_Memory;
		mappedRange.offset = p_Offset;
		mappedRange.size = p_SizeBytes;
		vkInvalidateMappedMemoryRanges(VulkanDevice::Get().GetDevice(), 1, &mappedRange);
#endif
	}

	void VulkanMemoryBuffer::Destroy(bool p_DeletionQueue) noexcept
	{
		YM_PROFILE_FUNCTION()

		auto buffer = m_Buffer;	
		

		if (p_DeletionQueue)
		{
	#ifdef USE_VMA_ALLOCATOR
			auto alloc = m_Allocation;
			VulkanContext::PushFunction([buffer, alloc]()
			{
				YM_CORE_TRACE("Destroying vulkan vertex buffer...")
				vmaDestroyBuffer(VulkanDevice::Get().GetAllocator(), buffer, alloc);
			});
	#else
			auto memory = m_Memory;
			VulkanContext::PushFunction([device, buffer, memory]()
			{
				YM_CORE_TRACE("Destroying vulkan vertex buffer")

				auto device = VulkanDevice::Get().GetDevice();

				if (buffer != VK_NULL_HANDLE)
					vkDestroyBuffer(device, buffer, VK_NULL_HANDLE);
				if (memory != VK_NULL_HANDLE)
					vkFreeMemory(device, memory, VK_NULL_HANDLE);
			});
	#endif
		}
		else
		{
			YM_CORE_TRACE("Destroying vulkan vertex buffer...")

	#ifdef USE_VMA_ALLOCATOR			
			vmaDestroyBuffer(VulkanDevice::Get().GetAllocator(), buffer, m_Allocation);
	#else
			auto device = VulkanDevice::Get().GetDevice();

			if (buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(device, buffer, VK_NULL_HANDLE);
			if (memory != VK_NULL_HANDLE)
				vkFreeMemory(device, memory, VK_NULL_HANDLE);
	#endif
		}
	}
}
