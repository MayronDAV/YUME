#pragma once

#include "YUME/Core/base.h"
#include <variant>



namespace YUME
{

	// TODO: Fill out


	enum class RenderType
	{
		None = 0
	};

	using RenderVariant = std::variant<std::monostate>;
	struct YM_LOCAL RenderPacket
	{
		RenderType Type; // Must to be defined
		RenderVariant Data = std::monostate{};
	};
}
