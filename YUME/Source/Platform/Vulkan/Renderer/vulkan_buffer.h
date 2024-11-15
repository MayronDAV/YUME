#pragma once
#include "YUME/Renderer/buffer.h"
#include "Platform/Vulkan/Core/vulkan_memory_buffer.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{

	class VulkanVertexBuffer : public VertexBuffer
	{
		public:
			VulkanVertexBuffer() = default;
			VulkanVertexBuffer(const void* p_Data, uint64_t p_SizeBytes);
			~VulkanVertexBuffer() override = default;

			void Bind(CommandBuffer* p_CommandBuffer) const override;
			void Unbind() const override;

			void SetData(const void* p_Data, uint64_t p_SizeBytes) override;

			void Flush() override;

		private:
			Unique<VulkanMemoryBuffer> m_Buffer;
	};


	class VulkanIndexBuffer : public IndexBuffer
	{
		public:
			VulkanIndexBuffer() = default;
			explicit VulkanIndexBuffer(const uint32_t* p_Indices, uint32_t p_Count);
			~VulkanIndexBuffer() override = default;

			void Bind(CommandBuffer* p_CommandBuffer) const override;
			void Unbind() const override;

			uint32_t GetCount() const override { return m_Count; }

		private:
			Unique<VulkanMemoryBuffer> m_Buffer;
			uint32_t m_Count = 0;
	};
}