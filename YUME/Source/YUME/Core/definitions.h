#pragma once
#include "YUME/Core/base.h"


namespace YUME
{
	// Var definitions

	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;


	enum class RenderAPI
	{
		Vulkan = 0, OpenGL, None
	};

	enum class CullMode : uint8_t
	{
		NONE = 0,
		FRONT,
		BACK,
		FRONTANDBACK
	};

	enum class PolygonMode : uint8_t
	{
		FILL = 0,
		LINE,
		POINT
	};

	enum class BlendMode : uint8_t
	{
		None = 0,
		OneZero,
		ZeroSrcColor,
		SrcAlphaOneMinusSrcAlpha,
		SrcAlphaOne
	};

	enum class DrawType : uint8_t
	{
		POINT = 0,
		TRIANGLE,
		LINES
	};

	enum class FrontFace : uint8_t
	{
		CLOCKWISE = 0,
		COUNTER_CLOCKWISE
	};

	enum class ShaderType
	{
		VERTEX = 0, FRAGMENT
	};
}