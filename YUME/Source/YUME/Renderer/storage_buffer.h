#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"



namespace YUME
{
	class YM_API StorageBuffer
	{
		public:
			virtual ~StorageBuffer() = default;

			virtual void SetData(void* p_Data, size_t p_SizeBytes, size_t p_Offset = 0) = 0;
			virtual void Fill(uint32_t p_Data) = 0;
			virtual void Fill(uint32_t p_Data, size_t p_SizeBytes) = 0;
			virtual void Resize(size_t p_SizeBytes) = 0;

			virtual size_t GetOffset() const = 0;
			virtual size_t GetSize() const = 0;

			static Ref<StorageBuffer> Create(size_t p_SizeBytes);
	};
}
