#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/definitions.h"


namespace YUME
{
	enum class DataType
	{
		None = 0, Float, Float2, Float3, Float4,
		Mat3, Mat4,
		UInt, UInt2,
		Int, Int2, Int3, Int4,
		Bool
	};

	static uint32_t DataTypeSize(DataType p_Type)
	{
		switch (p_Type)
		{
			case YUME::DataType::None:		return 0;

			case YUME::DataType::Float:		return 4;
			case YUME::DataType::Float2:	return 4 * 2;
			case YUME::DataType::Float3:	return 4 * 3;
			case YUME::DataType::Float4:	return 4 * 4;

			case YUME::DataType::Mat3:		return 4 * 3 * 3;
			case YUME::DataType::Mat4:		return 4 * 4 * 4;

			case YUME::DataType::UInt:		return 4;
			case YUME::DataType::UInt2:		return 4 * 2;

			case YUME::DataType::Int:		return 4;
			case YUME::DataType::Int2:		return 4 * 2;
			case YUME::DataType::Int3:		return 4 * 3;
			case YUME::DataType::Int4:		return 4 * 4;

			case YUME::DataType::Bool:		return 1;
		}

		YM_CORE_ASSERT(false)
		return 0;
	}


	struct YM_API BufferElement
	{
		std::string Name = " ";
		DataType Type = DataType::None;
		uint32_t Size = 0;
		uint32_t Offset = 0;
		bool Normalized = false;

		BufferElement() = default;
		BufferElement(DataType p_Type, const std::string& p_Name, bool normalized = false)
			:Name(p_Name), Type(p_Type), Size(DataTypeSize(p_Type)), Normalized(normalized)
		{}

		uint32_t GetComponentCount() const
		{
			switch (Type)
			{
				case YUME::DataType::None:		return 0;

				case YUME::DataType::Float:		return 1;
				case YUME::DataType::Float2:	return 2;
				case YUME::DataType::Float3:	return 3;
				case YUME::DataType::Float4:	return 4;

				case YUME::DataType::Mat3:		return 3 * 3;
				case YUME::DataType::Mat4:		return 4 * 4;

				case YUME::DataType::UInt:		return 1;
				case YUME::DataType::UInt2:		return 2;

				case YUME::DataType::Int:		return 1;
				case YUME::DataType::Int2:		return 2;
				case YUME::DataType::Int3:		return 3;
				case YUME::DataType::Int4:		return 4;

				case YUME::DataType::Bool:		return 1;
			}

			YM_CORE_ASSERT(false, "Unknown DataType!")
			return 0;
		}
	};


	class YM_API BufferLayout
	{
		public:
			BufferLayout() = default;
			BufferLayout(const std::initializer_list<BufferElement>& p_Element)
				: m_Elements(p_Element)
			{
				CalculateOffsetAndStride();
			}
			inline uint32_t GetStride() const { return m_Stride; }
			inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

			std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
			std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
			std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
			std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

		private:
			void CalculateOffsetAndStride()
			{
				uint32_t offset = 0;
				m_Stride = 0;
				for (auto& element : m_Elements)
				{
					element.Offset = offset;
					offset += element.Size;
					m_Stride += element.Size;
				}
			}

		private:
			std::vector<BufferElement> m_Elements;
			uint32_t m_Stride = 0;
	};


	class YM_API VertexBuffer
	{
		public:
			virtual ~VertexBuffer() = default;

			virtual void Bind() const = 0;
			virtual void Unbind() const = 0;

			virtual const BufferLayout& GetLayout() const = 0;
			virtual void SetLayout(const BufferLayout& p_Layout) = 0;

			virtual void SetData(const void* p_Data, uint64_t p_SizeBytes) = 0;

			virtual void Flush() {};

			static Ref<VertexBuffer> Create(const void* p_Data, uint64_t p_SizeBytes, BufferUsage p_Usage = BufferUsage::STATIC);
	};


	class YM_API IndexBuffer
	{
		public:
			virtual ~IndexBuffer() = default;

			virtual uint32_t GetCount() const = 0;

			virtual void Bind() const = 0;
			virtual void Unbind() const = 0;

			static Ref<IndexBuffer> Create(uint32_t* p_Indices, uint32_t p_Count);
	};
}

