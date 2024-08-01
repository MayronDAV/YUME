#pragma once
#include "YUME/Renderer/uniform_buffer.h"

// Lib
#include <vulkan/vulkan.h>


namespace YUME
{
	class YM_API VulkanUniformBuffer : public UniformBuffer
	{
		public:
			VulkanUniformBuffer(const void* p_Data, uint32_t p_SizeBytes, uint32_t p_Offset = 0, uint32_t p_Binding = 0);
			VulkanUniformBuffer(uint32_t p_SizeBytes, uint32_t p_Offset = 0, uint32_t p_Binding = 0);
			~VulkanUniformBuffer() override;

			void SetData(const void* p_Data, uint32_t p_SizeBytes) override;
			
		private:
			uint32_t m_Binding = 0;
			uint32_t m_Offset = 0;
			uint32_t m_SizeBytes = 0;

			VkBuffer m_Buffer = VK_NULL_HANDLE;
			VkDeviceMemory m_Memory = VK_NULL_HANDLE;
			VkMemoryRequirements m_Requirements;
	};
}