#pragma once

#include "YUME/Core/base.h"
#include <xhash>



namespace YUME
{
	class YM_API UUID
	{
		public:
			UUID();
			UUID(uint64_t p_uuid);
			UUID(const UUID&) = default;

			operator uint64_t() const { return m_UUID; }

		private:
			uint64_t m_UUID;
	};

}


namespace std
{
	template<>
	struct hash<YUME::UUID>
	{
		std::size_t operator()(const YUME::UUID& p_uuid) const
		{
			return (uint64_t)p_uuid;
		}
	};
}
