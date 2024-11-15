#include "YUME/yumepch.h"
#include "vulkan_buffer.h"

#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_context.h"
#include "YUME/Core/application.h"



namespace YUME
{

	VulkanVertexBuffer::VulkanVertexBuffer(const void* p_Data, uint64_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		if (p_Data != nullptr)
		{
			YM_CORE_VERIFY(p_Data != nullptr)

			m_Buffer = CreateUnique<VulkanMemoryBuffer>(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				p_SizeBytes
			);

			m_Buffer->SetData(p_SizeBytes, p_Data);
		}
		else
		{
			m_Buffer = CreateUnique<VulkanMemoryBuffer>(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				p_SizeBytes
			);
		}
	}

	void VulkanVertexBuffer::Bind(CommandBuffer* p_CommandBuffer) const
	{
		YM_PROFILE_FUNCTION()

		auto commandBuffer = static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle();

		VkDeviceSize offset = 0;
		auto buffer = m_Buffer->GetBuffer();
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
	}

	void VulkanVertexBuffer::Unbind() const
	{
		YM_PROFILE_FUNCTION()

	}

	void VulkanVertexBuffer::SetData(const void* p_Data, uint64_t p_SizeBytes)
	{
		m_Buffer->SetData(p_SizeBytes, p_Data);
	}

	void VulkanVertexBuffer::Flush()
	{
		m_Buffer->Flush();
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* p_Indices, uint32_t p_Count)
		: m_Count(p_Count)
	{
		YM_PROFILE_FUNCTION()

		uint64_t sizeBytes = p_Count * sizeof(uint32_t);
		m_Buffer = CreateUnique<VulkanMemoryBuffer>(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeBytes
		);

		m_Buffer->SetData(sizeBytes, p_Indices);
	}


	void VulkanIndexBuffer::Bind(CommandBuffer* p_CommandBuffer) const
	{
		YM_PROFILE_FUNCTION()

		auto commandBuffer = static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle();

		VkDeviceSize offset = 0;
		auto buffer = m_Buffer->GetBuffer();
		vkCmdBindIndexBuffer(commandBuffer, buffer, offset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanIndexBuffer::Unbind() const
	{
		YM_PROFILE_FUNCTION()

	}
}
