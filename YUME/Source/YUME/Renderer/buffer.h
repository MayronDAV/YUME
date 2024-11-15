#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Core/definitions.h"
#include "YUME/Core/command_buffer.h"


namespace YUME
{
	class YM_API VertexBuffer
	{
		public:
			virtual ~VertexBuffer() = default;

			virtual void Bind(CommandBuffer* p_CommandBuffer) const = 0;
			virtual void Unbind() const = 0;

			virtual void SetData(const void* p_Data, uint64_t p_SizeBytes) = 0;

			virtual void Flush() {};

			static Ref<VertexBuffer> Create(const void* p_Data, uint64_t p_SizeBytes);
	};


	class YM_API IndexBuffer
	{
		public:
			virtual ~IndexBuffer() = default;

			virtual uint32_t GetCount() const = 0;

			virtual void Bind(CommandBuffer* p_CommandBuffer) const = 0;
			virtual void Unbind() const = 0;

			static Ref<IndexBuffer> Create(const uint32_t* p_Indices, uint32_t p_Count);
	};
}

