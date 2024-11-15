#include "YUME/yumepch.h"
#include "vulkan_storage_buffer.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"



namespace YUME
{
	VulkanStorageBuffer::VulkanStorageBuffer(size_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		m_Buffer = CreateUnique<VulkanMemoryBuffer>(
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			p_SizeBytes
		);
	}

	void VulkanStorageBuffer::SetData(void* p_Data, size_t p_SizeBytes, size_t p_Offset)
	{
		YM_PROFILE_FUNCTION()

		m_Offset = p_Offset;
		m_Size = p_SizeBytes;
		m_Buffer->SetData(p_SizeBytes, p_Data, p_Offset);
	}

	void VulkanStorageBuffer::Fill(uint32_t p_Data)
	{
		YM_PROFILE_FUNCTION()

		m_Buffer->Fill(p_Data, m_Size);
	}

	void VulkanStorageBuffer::Fill(uint32_t p_Data, size_t p_SizeBytes)
	{
		m_Buffer->Fill(p_Data, p_SizeBytes);
	}

	void VulkanStorageBuffer::Resize(size_t p_SizeBytes)
	{
		if (p_SizeBytes > 0 && p_SizeBytes != m_Size)
		{
			m_Buffer->Resize(p_SizeBytes);
		}
	}
}
