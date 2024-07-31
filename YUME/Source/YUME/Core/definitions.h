#pragma once
#include "YUME/Core/base.h"


namespace YUME
{
	// Var definitions
	static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;
	static constexpr uint8_t MAX_MIPS = 32;

	// Descriptor set limits
	static constexpr uint16_t DESCRIPTOR_MAX_STORAGE_TEXTURES = 1024;
	static constexpr uint16_t DESCRIPTOR_MAX_STORAGE_BUFFERS = 1024;
	static constexpr uint16_t DESCRIPTOR_MAX_CONSTANT_BUFFERS = 1024;
	static constexpr uint16_t DESCRIPTOR_MAX_CONSTANT_BUFFERS_DYNAMIC = 1024;
	static constexpr uint16_t DESCRIPTOR_MAX_SAMPLERS = 1024;
	static constexpr uint16_t DESCRIPTOR_MAX_TEXTURES = 1024;


	enum class RenderAPI : uint8_t
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

	enum class ShaderType : uint8_t
	{
		VERTEX = 0, FRAGMENT
	};

	enum TextureFlagBits : uint8_t
	{
		TEXTURE_SAMPLED = BIT(0),
		TEXTURE_STORAGE = BIT(1),
		TEXTURE_RENDER_TARGET = BIT(2),
		TEXTURE_DEPTH_STENCIL = BIT(3),
		TEXTURE_DEPTH_STENCIL_READONLY = BIT(4),
		TEXTURE_CREATE_MIPS = BIT(5),
		TEXTURE_MIP_VIEWS = BIT(6)
	};
	using TextureFlags = uint16_t;

	enum class TextureWrap : uint8_t
	{
		NONE = 0,
		REPEAT,
		CLAMP,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	enum class TextureBorderColor : uint8_t
	{
		// FLOAT -> 0.0f to 1.0f
		// SRGB -> 0 to 255

		TRANSPARENT_BLACK_FLOAT = 0,
		TRANSPARENT_BLACK_SRGB,
		OPAQUE_BLACK_FLOAT,
		OPAQUE_BLACK_SRGB,
		OPAQUE_WHITE_FLOAT,
		OPAQUE_WHITE_SRGB,
		CUSTOM_FLOAT,
		CUSTOM_SRGB
	};

	enum class TextureFilter : uint8_t
	{
		NONE = 0,
		LINEAR,
		NEAREST
	};

	enum class TextureFormat : uint8_t
	{
		None = 0,

		// Color Format

		R8_SRGB,
		R8_INT,
		R8_UINT,
		RGB8_SRGB,
		RGBA8_SRGB,
		RGBA32_FLOAT,
		R32_INT,
		RG32_UINT,

		// Depth Format

		D16_NORM, // -1.0 (-32768 ) to 1.0 (32767)
		D32_FLOAT,

		// DepthStencil Format

		D16_UNORM_S8_UINT,
		D24_UNORM_S8_UINT,
		D32_FLOAT_S8_UINT,
	};

}