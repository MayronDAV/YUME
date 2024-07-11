#include "YUME/yumepch.h"
#include "window.h"

// TODO: make a settings manager for choose vulkan, opengl e etc...
// Currently only support Vulkan

#include "Platforms/Vulkan/Core/vulkan_window.h"



namespace YUME
{
	Window* Window::Create(const WindowProps& p_Props)
	{
		return new VULKAN::Window(p_Props);
	}
}
