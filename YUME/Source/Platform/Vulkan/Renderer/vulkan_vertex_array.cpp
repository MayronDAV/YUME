#include "YUME/yumepch.h"
#include "vulkan_vertex_array.h"
#include "vulkan_context.h"



namespace YUME
{
	namespace Utils
	{
		static VkFormat DataTypeToVkFormat(DataType p_Type)
		{
			switch (p_Type)
			{
				case YUME::DataType::Float:  return VK_FORMAT_R32_SFLOAT;
				case YUME::DataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
				case YUME::DataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
				case YUME::DataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;

				case YUME::DataType::Mat3:   return VK_FORMAT_R32G32B32_SFLOAT;
				case YUME::DataType::Mat4:   return VK_FORMAT_R32G32B32A32_SFLOAT;

				case YUME::DataType::UInt:   return VK_FORMAT_R32_UINT;
				case YUME::DataType::UInt2:  return VK_FORMAT_R32G32_UINT;

				case YUME::DataType::Int:    return VK_FORMAT_R32_SINT;
				case YUME::DataType::Int2:   return VK_FORMAT_R32G32_SINT;
				case YUME::DataType::Int3:   return VK_FORMAT_R32G32B32_SINT;
				case YUME::DataType::Int4:   return VK_FORMAT_R32G32B32A32_SINT;

				case YUME::DataType::Bool:   return VK_FORMAT_R8_UINT;
			}

			YM_CORE_ASSERT(false, "Unknown DataType!")
				return (VkFormat)0;
		}
	}

	uint32_t VulkanVertexArray::s_VertexBufferBinding = 0;


	VulkanVertexArray::~VulkanVertexArray() 
	{
		for (auto& vertexBuffer : m_VertexBuffers)
		{
			if (vertexBuffer != nullptr)
				vertexBuffer.reset();
		}

		if (m_IndexBuffer != nullptr)
			m_IndexBuffer.reset();
	};

	void VulkanVertexArray::Bind() const
	{
		for (const auto& vertexBuffer : m_VertexBuffers)
		{
			vertexBuffer->Bind();
		}
		if (m_IndexBuffer != nullptr)
			m_IndexBuffer->Bind();
	}

	void VulkanVertexArray::Unbind() const
	{
		for (const auto& vertexBuffer : m_VertexBuffers)
		{
			vertexBuffer->Unbind();
		}
		if (m_IndexBuffer != nullptr)
			m_IndexBuffer->Unbind();
	}

	void VulkanVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& p_VertexBuffer)
	{
		YM_PROFILE_FUNCTION()

		const auto& layout = p_VertexBuffer->GetLayout();

		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = s_VertexBufferBinding;
		bindingDescription.stride = layout.GetStride();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		for (const auto& element : layout)
		{
			switch (element.Type)
			{
				case DataType::Mat3:
				{
					for (int i = 0; i < 3; i++)
					{
						VkVertexInputAttributeDescription attDesc;

						attDesc.location = m_VertexBufferLocation;
						attDesc.binding = s_VertexBufferBinding;
						attDesc.format = Utils::DataTypeToVkFormat(element.Type);
						attDesc.offset = element.Size * i;

						m_VertexBufferLocation++;
						m_AttributeDescs.push_back(attDesc);
					}
					break;
				}
				case DataType::Mat4:
				{
					for (int i = 0; i < 4; i++)
					{
						VkVertexInputAttributeDescription attDesc;

						attDesc.location = m_VertexBufferLocation;
						attDesc.binding = s_VertexBufferBinding;
						attDesc.format = Utils::DataTypeToVkFormat(element.Type);
						attDesc.offset = element.Size * i;

						m_VertexBufferLocation++;
						m_AttributeDescs.push_back(attDesc);
					}
					break;
				}
				default:
				{
					VkVertexInputAttributeDescription attDesc;

					attDesc.location = m_VertexBufferLocation;
					attDesc.binding = s_VertexBufferBinding;
					attDesc.format = Utils::DataTypeToVkFormat(element.Type);
					attDesc.offset = element.Offset;

					m_VertexBufferLocation++;
					m_AttributeDescs.push_back(attDesc);
					break;
				}
			}
		}

		s_VertexBufferBinding++;
		m_BindingDescs.push_back(bindingDescription);
		m_VertexBuffers.push_back(p_VertexBuffer);
	}

	void VulkanVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& p_IndexBuffer)
	{
		m_IndexBuffer = p_IndexBuffer;
	}
}
