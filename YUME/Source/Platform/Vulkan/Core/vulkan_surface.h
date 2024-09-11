#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/singleton.h"

// Lib
#include <vulkan/vulkan.h>


namespace YUME
{
	// Maybe change this to platform os instead of glfw.

	class VulkanSurface : public ThreadSafeSingleton<VulkanSurface>
	{
		friend class ThreadSafeSingleton<VulkanSurface>;

		public:
			VulkanSurface() = default;
			~VulkanSurface();

			void Init(void* p_Window);

			VkSurfaceKHR& GetSurface() { return m_Surface; }

		private:
			VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	};
}