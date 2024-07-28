#pragma once
#include "YUME/Renderer/buffer.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{

	class YM_API VulkanVertexBuffer : public VertexBuffer
	{
		public:
			VulkanVertexBuffer() = default;
			explicit VulkanVertexBuffer(uint64_t p_SizeBytes);
			VulkanVertexBuffer(const float* p_Vertices, uint64_t p_SizeBytes);
			~VulkanVertexBuffer() override;

			void Bind() const override;
			void Unbind() const override;

			const BufferLayout& GetLayout() const override { return m_Layout; }
			void SetLayout(const BufferLayout& p_Layout) override { m_Layout = p_Layout; }

			void SetData(const void* p_Data, uint64_t p_SizeBytes) override;

		private:
			VkBuffer m_Buffer = VK_NULL_HANDLE;
			VkDeviceMemory m_Memory = VK_NULL_HANDLE;
			VkMemoryRequirements m_Requirements{};

			BufferLayout m_Layout{};
			uint32_t m_Binding = -1;
			uint64_t m_SizeBytes = 0;

			VkVertexInputBindingDescription m_BindingDesc{};
			std::vector<VkVertexInputAttributeDescription> m_AttributeDescs;
	};


	class YM_API VulkanIndexBuffer : public IndexBuffer
	{
		public:
			VulkanIndexBuffer() = default;
			explicit VulkanIndexBuffer(const uint32_t* p_Indices, uint32_t p_Count);
			~VulkanIndexBuffer() override;

			void Bind() const override;
			void Unbind() const override;

			uint32_t GetCount() const override { return m_Count; }

		private:
			VkBuffer m_Buffer = VK_NULL_HANDLE;
			VkDeviceMemory m_Memory = VK_NULL_HANDLE;
			VkMemoryRequirements m_Requirements{};

			uint32_t m_Count = 0;
	};
}