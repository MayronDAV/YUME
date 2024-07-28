#include "YUME/yumepch.h"
#include "vulkan_surface.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace YUME
{
	VulkanSurface::~VulkanSurface()
	{
		YM_CORE_TRACE("Destroying vulkan surface...")
		vkDestroySurfaceKHR(VulkanContext::GetInstance(), m_Surface, VK_NULL_HANDLE);
	}

	void VulkanSurface::Init(void* p_Window)
	{
		auto glfwWindow = (GLFWwindow*)p_Window;
		auto res = glfwCreateWindowSurface(VulkanContext::GetInstance(), glfwWindow, VK_NULL_HANDLE, &m_Surface);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}
}
