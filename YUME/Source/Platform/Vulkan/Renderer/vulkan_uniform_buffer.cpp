#include "YUME/yumepch.h"
#include "vulkan_uniform_buffer.h"

#include "vulkan_context.h"
#include "Platform/Vulkan/Core/vulkan_device.h"




namespace YUME
{
	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		m_Buffer = CreateScope<VulkanMemoryBuffer>(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			p_SizeBytes
		);
	}

	VulkanUniformBuffer::VulkanUniformBuffer(const void* p_Data, uint32_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		m_Buffer = CreateScope<VulkanMemoryBuffer>(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			p_SizeBytes
		);

		m_Buffer->SetData(p_SizeBytes, p_Data, 0);
	}

	void VulkanUniformBuffer::SetData(const void* p_Data, uint32_t p_SizeBytes, uint32_t p_Offset)
	{
		YM_PROFILE_FUNCTION()

		m_Offset = p_Offset;
		m_Buffer->SetData(p_SizeBytes, p_Data, (uint64_t)p_Offset);
	}
}
