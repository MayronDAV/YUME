#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/buffer.h"


namespace YUME
{
	class YM_API VertexArray
	{
		public:
			virtual ~VertexArray() = default;

			virtual void Bind() const = 0;
			virtual void Unbind() const = 0;

			virtual void AddVertexBuffer(const Ref<VertexBuffer>& p_VertexBuffer) = 0;
			virtual void SetIndexBuffer(const Ref<IndexBuffer>& p_IndexBuffer) = 0;

			uint32_t GetIndexCount() const { return GetIndexBuffers()->GetCount(); }

			virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
			virtual const Ref<IndexBuffer>& GetIndexBuffers() const = 0;

			static Ref<VertexArray> Create();
	};
}

