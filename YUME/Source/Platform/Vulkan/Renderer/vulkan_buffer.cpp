#include "YUME/yumepch.h"
#include "vulkan_buffer.h"

#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_context.h"
#include "YUME/Core/application.h"



namespace YUME
{

	VulkanVertexBuffer::VulkanVertexBuffer(const void* p_Data, uint64_t p_SizeBytes, BufferUsage p_Usage)
	{
		YM_PROFILE_FUNCTION()

		if (p_Usage == BufferUsage::STATIC)
		{
			YM_CORE_VERIFY(p_Data != nullptr)

			m_Buffer = CreateScope<VulkanMemoryBuffer>(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				p_SizeBytes
			);

			m_Buffer->SetData(p_SizeBytes, p_Data);
		}
		else if (p_Usage == BufferUsage::DYNAMIC)
		{
			m_Buffer = CreateScope<VulkanMemoryBuffer>(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				p_SizeBytes
			);
		}
		else
		{
			YM_CORE_ERROR("Unknwon buffer usage!")
		}
	}

	void VulkanVertexBuffer::Bind() const
	{
		YM_PROFILE_FUNCTION()

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		VkDeviceSize offset = 0;
		auto buffer = m_Buffer->GetBuffer();
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer, &offset);
	}

	void VulkanVertexBuffer::Unbind() const
	{
		YM_PROFILE_FUNCTION()

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		VkBuffer nullBuffer = VK_NULL_HANDLE;
		VkDeviceSize nullOffset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &nullBuffer, &nullOffset);
	}

	void VulkanVertexBuffer::SetData(const void* p_Data, uint64_t p_SizeBytes)
	{
		m_Buffer->SetData(p_SizeBytes, p_Data);
	}



	VulkanIndexBuffer::VulkanIndexBuffer(const uint32_t* p_Indices, uint32_t p_Count)
		: m_Count(p_Count)
	{
		YM_PROFILE_FUNCTION()

		uint64_t sizeBytes = p_Count * sizeof(uint32_t);
		m_Buffer = CreateScope<VulkanMemoryBuffer>(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			sizeBytes
		);

		m_Buffer->SetData(sizeBytes, p_Indices);
	}


	void VulkanIndexBuffer::Bind() const
	{
		YM_PROFILE_FUNCTION()

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		VkDeviceSize offset = 0;
		auto buffer = m_Buffer->GetBuffer();
		vkCmdBindIndexBuffer(commandBuffer, buffer, offset, VK_INDEX_TYPE_UINT32);
	}
	void VulkanIndexBuffer::Unbind() const
	{
		YM_PROFILE_FUNCTION()

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		VkDeviceSize offset = 0;
		VkBuffer nullBuffer = VK_NULL_HANDLE;
		vkCmdBindIndexBuffer(commandBuffer, nullBuffer, offset, VK_INDEX_TYPE_UINT32);
	}
}
