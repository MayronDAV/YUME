#pragma once

// TODO: Change this later


namespace YUME
{
	enum class RenderAPI
	{
		None = 0, OpenGL, Vulkan
	};

	static const RenderAPI s_RenderAPI = RenderAPI::Vulkan;
}
