#pragma once
#include "YUME/Core/base.h"



namespace YUME
{
	class YM_API UniformBuffer
	{
		public:
			virtual ~UniformBuffer() = default;
			virtual void SetData(const void* p_Data, uint32_t p_SizeBytes) = 0;

			static Ref<UniformBuffer> Create(uint32_t p_SizeBytes, uint32_t p_Offset = 0, uint32_t p_Binding = 0);
			static Ref<UniformBuffer> Create(const void* p_Data, uint32_t p_SizeBytes, uint32_t p_Offset = 0, uint32_t p_Binding = 0);
	};
}