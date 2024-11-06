#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "pipeline.h"
#include "uniform_buffer.h"
#include "YUME/Core/definitions.h"

// std
#include <string_view>

// Lib
#include <glm/glm.hpp>
#include "texture.h"



namespace YUME
{
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

	struct YM_API InputElement
	{
		std::string Name = " ";
		DataType Type = DataType::None;
		uint32_t Size = 0;
		uint32_t Offset = 0;
		bool Normalized = false;

		InputElement() = default;
		InputElement(DataType p_Type, const std::string& p_Name, bool p_Normalized = false)
			: Name(p_Name), Type(p_Type), Size(DataTypeSize(p_Type)), Normalized(p_Normalized)
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

	class YM_API InputLayout
	{
		public:
			InputLayout() = default;
			InputLayout(const std::initializer_list<InputElement>& p_Element)
				: m_Elements(p_Element)
			{
				CalculateOffsetAndStride();
			}
			inline uint32_t GetStride() const { return m_Stride; }
			inline const std::vector<InputElement>& GetElements() const { return m_Elements; }

			std::vector<InputElement>::iterator begin() { return m_Elements.begin(); }
			std::vector<InputElement>::iterator end() { return m_Elements.end(); }
			std::vector<InputElement>::const_iterator begin() const { return m_Elements.begin(); }
			std::vector<InputElement>::const_iterator end() const { return m_Elements.end(); }

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
			std::vector<InputElement> m_Elements;
			uint32_t m_Stride = 0;
	};

	class YM_API Shader
	{
		public:
			virtual ~Shader() = default;

			virtual void Bind() = 0;
			virtual void Unbind() = 0;
		
			virtual const std::string_view& GetName() const = 0;

			virtual void SetLayout(const InputLayout& p_Layout) = 0;

			virtual void SetPushValue(const std::string& p_Name, void* p_Value) = 0;
			virtual void BindPushConstants() const = 0;

			static Ref<Shader> Create(const std::string& p_ShaderPath);
	};
}