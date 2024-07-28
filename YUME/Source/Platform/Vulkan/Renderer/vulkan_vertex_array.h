#pragma once
#include "YUME/Renderer/vertex_array.h"
#include "vulkan_buffer.h"

#include <vulkan/vulkan.h>


namespace YUME
{
	using BindingDescription = std::vector<VkVertexInputBindingDescription>;
	using AttributeDescription = std::vector<VkVertexInputAttributeDescription>;

	class YM_API VulkanVertexArray : public VertexArray
	{
		public:
			VulkanVertexArray() = default;
			~VulkanVertexArray() override;

			void Bind() const override;
			void Unbind() const override;

			void AddVertexBuffer(const Ref<VertexBuffer>& p_VertexBuffer) override;
			void SetIndexBuffer(const Ref<IndexBuffer>& p_IndexBuffer) override;

			const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
			const Ref<IndexBuffer>& GetIndexBuffers() const override { return m_IndexBuffer; }

			const AttributeDescription& GetAttributeDescription() const { return m_AttributeDescs; }
			const BindingDescription& GetBindingDescription() const { return m_BindingDescs; }

		private:
			uint32_t m_VertexBufferLocation = 0;
			static uint32_t s_VertexBufferBinding;

			std::vector<Ref<VertexBuffer>> m_VertexBuffers;
			Ref<IndexBuffer> m_IndexBuffer = nullptr;

			AttributeDescription m_AttributeDescs;
			BindingDescription m_BindingDescs;
	};
}