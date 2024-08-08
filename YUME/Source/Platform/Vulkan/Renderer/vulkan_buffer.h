#pragma once
#include "YUME/Renderer/buffer.h"
#include "Platform/Vulkan/Core/vulkan_memory_buffer.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{

	class YM_API VulkanVertexBuffer : public VertexBuffer
	{
		public:
			VulkanVertexBuffer() = default;
			VulkanVertexBuffer(const void* p_Data, uint64_t p_SizeBytes, BufferUsage p_Usage = BufferUsage::STATIC);
			~VulkanVertexBuffer() override = default;

			void Bind() const override;
			void Unbind() const override;

			const BufferLayout& GetLayout() const override { return m_Layout; }
			void SetLayout(const BufferLayout& p_Layout) override { m_Layout = p_Layout; }

			void SetData(const void* p_Data, uint64_t p_SizeBytes) override;

			void Flush() override;

		private:
			Scope<VulkanMemoryBuffer> m_Buffer;
			BufferLayout m_Layout{};
	};


	class YM_API VulkanIndexBuffer : public IndexBuffer
	{
		public:
			VulkanIndexBuffer() = default;
			explicit VulkanIndexBuffer(const uint32_t* p_Indices, uint32_t p_Count);
			~VulkanIndexBuffer() override = default;

			void Bind() const override;
			void Unbind() const override;

			uint32_t GetCount() const override { return m_Count; }

		private:
			Scope<VulkanMemoryBuffer> m_Buffer;
			uint32_t m_Count = 0;
	};
}