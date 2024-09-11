#pragma once
#include "YUME/Core/base.h"

// std
#include <stdint.h>
#include <atomic>



namespace YUME
{
	struct YM_API ReferenceCounter
	{
			std::atomic<int> Count = 0;

		public:
			inline bool Ref()
			{
				Count++;
				return Count != 0;
			}

			inline int RefValue()
			{
				Count++;
				return Count;
			}

			inline bool Unref()
			{
				--Count;
				bool del = (Count == 0);
				return del;
			}

			inline int Get() const
			{
				return Count;
			}

			inline void Init(int p_Value = 1)
			{
				Count = p_Value;
			}
	};
}
