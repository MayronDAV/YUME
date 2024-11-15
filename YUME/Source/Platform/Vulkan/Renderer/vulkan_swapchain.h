#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/singleton.h"
#include "YUME/Renderer/texture.h"
#include "Platform/Vulkan/Core/vulkan_commandpool.h"
#include "Platform/Vulkan/Core/vulkan_command_buffer.h"
#include "Platform/Vulkan/Core/vulkan_sync.h"
#include "YUME/Core/reference.h"



namespace YUME
{
	struct FrameData
	{
		Unique<VulkanSemaphore> ImageAcquireSemaphore;
		Unique<VulkanCommandPool> CommandPool;
		Unique<VulkanCommandBuffer> MainCommandBuffer;
	};


	class VulkanSwapchain : public ThreadSafeSingleton<VulkanSwapchain>
	{
		friend class ThreadSafeSingleton<VulkanSwapchain>;

		public:
			VulkanSwapchain() = default;
			~VulkanSwapchain();

			void OnResize(uint32_t p_Width, uint32_t p_Height, bool p_Force = false, void* p_WindowHandle = nullptr);

			void Init(bool p_Vsync, void* p_Window, VkExtent2D p_Extent = {0, 0});

			void AcquireNextImage();
			void Present(const std::vector<VulkanSemaphore>& p_WaitSemaphores = {});
			void QueueSubmit();
			void Begin();
			void End();

			void SetVsync(bool p_Enable);
			
			VkSurfaceKHR& GetSurface() { return m_Surface; }
			VkSwapchainKHR& GetSwapChain() { return m_SwapChain; }
			VkExtent2D GetExtent2D() const { return m_Extent2D; }
			VkSurfaceFormatKHR GetFormat() const { return m_Format; }

			const FrameData& GetCurrentFrameData() const { return m_Frames[m_CurrentBuffer]; }

			uint32_t GetBufferCount() const { return m_BufferCount; }
			uint32_t GetCurrentBuffer() const { return m_CurrentBuffer; }
			uint32_t GetImageIndex() const { return m_AcquireImageIndex; }
			const Ref<Texture2D>* GetBuffers() const { return m_Buffers; }

		private:
			void ChooseSwapExtent2D(void* p_Window);
			void ChooseSurfaceFormat();
			void CreateFrameData();

		private:
			FrameData				 m_Frames[MAX_SWAPCHAIN_BUFFERS];
			Ref<Texture2D>			 m_Buffers[MAX_SWAPCHAIN_BUFFERS];
			std::vector<VkImageView> m_ImageViews;

			bool	 m_Vsync				  = false;
			uint32_t m_CurrentBuffer		  = 0;
			uint32_t m_BufferCount			  = 0;
			uint32_t m_AcquireImageIndex	  = 0;

			VkSurfaceKHR	   m_Surface	  = VK_NULL_HANDLE;
			VkSwapchainKHR	   m_OldSwapChain = VK_NULL_HANDLE;
			VkSwapchainKHR	   m_SwapChain	  = VK_NULL_HANDLE;
			VkSurfaceFormatKHR m_Format		  = { .format = VK_FORMAT_R8G8B8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			VkPresentModeKHR   m_PresentMode  = VK_PRESENT_MODE_IMMEDIATE_KHR;
			VkExtent2D		   m_Extent2D	  = { 0, 0 };
	};
}