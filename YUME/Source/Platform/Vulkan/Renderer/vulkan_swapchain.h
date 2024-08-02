#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/singleton.h"
#include "Platform/Vulkan/Core/vulkan_surface.h"
#include "Platform/Vulkan/Core/vulkan_commandpool.h"



namespace YUME
{

	class YM_API VulkanSwapchain : public ThreadSafeSingleton<VulkanSwapchain>
	{
		friend class ThreadSafeSingleton<VulkanSwapchain>;

		public:
			VulkanSwapchain() = default;
			~VulkanSwapchain();

			void CleanUp();
			void Invalidade(uint32_t p_Width, uint32_t p_Height);

			void Init(bool p_Vsync, void* p_Window, VkExtent2D p_Extent = {0, 0});

			VkSwapchainKHR GetSwapChain() { return m_SwapChain; }
			VkExtent2D GetExtent2D() const { return m_Extent2D; }
			VkSurfaceFormatKHR GetFormat() const { return m_Format; }

			std::vector<VkImage>& GetImages() { return m_Images; }
			std::vector<VkImageView>& GetImageViews() { return m_ImageViews; }

			void SetVsync(bool p_Enable) { if (p_Enable != m_Vsync) { m_Vsync = p_Enable; Invalidade(m_Extent2D.width, m_Extent2D.height); } }

		private:
			void ChooseSwapExtent2D(void* p_Window);
			void ChoosePresentMode(bool p_Vsync);
			void CheckSurfaceFormat();

		private:
			VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;

			VkSurfaceFormatKHR m_Format = { .format = VK_FORMAT_R8G8B8A8_SRGB, .colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR };
			bool m_VsyncIsEnable = false; // try to use VK_PRESENT_MODE_MAILBOX_KHR, if not available use VK_PRESENT_MODE_FIFO_KHR
			VkPresentModeKHR m_PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			VkExtent2D m_Extent2D{};

			std::vector<VkImage> m_Images;
			std::vector<VkImageView> m_ImageViews;

			void* m_Window = nullptr;
			bool m_Vsync = false;
	};
}