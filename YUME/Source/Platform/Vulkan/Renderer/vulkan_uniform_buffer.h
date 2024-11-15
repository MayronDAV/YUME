#pragma once
#include "YUME/Renderer/uniform_buffer.h"
#include "Platform/Vulkan/Core/vulkan_memory_buffer.h"

// Lib
#include <vulkan/vulkan.h>


namespace YUME
{
	class VulkanUniformBuffer : public UniformBuffer
	{
		public:
			VulkanUniformBuffer(uint32_t p_SizeBytes);
			VulkanUniformBuffer(const void* p_Data, uint32_t p_SizeBytes);
			~VulkanUniformBuffer() override = default;

			void SetData(const void* p_Data, uint32_t p_SizeBytes, uint32_t p_Offset = 0) override;
			
			uint32_t GetOffset() const override { return m_Offset; }
			uint32_t GetSizeBytes() const { return m_SizeBytes; }
			VkBuffer GetBuffer() { return m_Buffer->GetBuffer(); }

		private:
			uint32_t m_Offset = 0;
			uint32_t m_SizeBytes = 0;

			Unique<VulkanMemoryBuffer> m_Buffer;
	};
}