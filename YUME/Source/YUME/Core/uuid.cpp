#include "YUME/yumepch.h"
#include "uuid.h"

#include <random>


namespace YUME
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistributions;


	UUID::UUID()
		: m_UUID(s_UniformDistributions(s_Engine))
	{
	}

	UUID::UUID(uint64_t p_uuid)
		: m_UUID(p_uuid)
	{
	}
}