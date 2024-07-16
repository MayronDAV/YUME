#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/graphics_context.h"

// Lib
#include <vulkan/vulkan.h>



struct GLFWwindow;


namespace YUME
{
	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	struct PhysicalDevice
	{
		VkPhysicalDevice Device;
		VkPhysicalDeviceProperties Properties;
		VkPhysicalDeviceFeatures Features;
		std::vector<VkQueueFamilyProperties> FamilyProperties;
		std::vector<VkBool32> QueueSupportsPresent;
		std::vector<VkSurfaceFormatKHR> SurfaceFormats;
		VkSurfaceCapabilitiesKHR SurfaceCapabilities;
		VkPhysicalDeviceMemoryProperties MemoryProperties;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class YM_API VulkanContext : public GraphicsContext
	{
		public:
			~VulkanContext() override;

			void Init(void* p_Window) override;

			void SwapBuffer() override;

			struct YM_API SwapchainOptions
			{
				VkPresentModeKHR PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				VkExtent2D Extent2D = { 800, 600 };
				VkSurfaceFormatKHR Format = { VK_FORMAT_R8G8B8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
				VkImageUsageFlags Usage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
			};

			VkSwapchainKHR& CreateSwapchain(const SwapchainOptions& p_Options = {});

			struct YM_API ImagesViewOptions
			{
				VkImageViewType ViewType = VK_IMAGE_VIEW_TYPE_2D;
				VkSurfaceFormatKHR Format = { VK_FORMAT_R8G8B8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
				VkComponentMapping Component = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY
				};
				VkImageSubresourceRange SubresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				};
			};

			std::vector<VkImageView>& CreateImagesView(const ImagesViewOptions& p_Options = {});

			VkRenderPass& CreateRenderPass(const VkRenderPassCreateInfo* p_Info);

			std::vector<VkFramebuffer>& CreateSwapchainFramebuffer();

			void RecreateSwapchain();

			void OnResize() { m_ViewportResized = true; }


			// Sync Objs
			void WaitFence();
			void ResetFence();

			// Command pool and Command buffer
			VkCommandPool& CreateCommandPool(const VkCommandPoolCreateInfo* p_Info);

			void HasDrawCommands(bool p_Has) { m_HasDrawCommands = p_Has; }

			void TransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout);

			// Draw
			void BeginFrame();

			VkSemaphore& GetSignalSemaphore() { return m_SignalSemaphores[m_CurrentFrame]; }
			VkSemaphore& GetWaitSemaphore() { return m_WaitSemaphores[m_CurrentFrame]; }

			VkFence& GetInFlightFence() { return m_InFlightFences[m_CurrentFrame]; }
			VkCommandBuffer& GetCommandBuffer() { return m_CommandBuffers[m_CurrentFrame]; }
			VkRenderPass& GetRenderPass() { return m_RenderPass; }
			VkFramebuffer& GetSwapchainFramebuffer() { return m_SwapchainFramebuffers[m_CurrentImageIndex]; }
			VkExtent2D GetCurrentExtent2D() { return m_SwapchainOptions.Extent2D; }
			VkQueue& GetQueue() { return m_Queue; }
			VkImage& GetImage() { return m_Images[m_CurrentImageIndex]; }
			VkDevice& GetDevice() { return m_Device; }

		private:
			void CreateInstance();
			void GetPhysicalDevices();
			void SelectPhysicalDevice();
			void CreateLogicalDevice();
			void CreateSyncObjs();

			void ResetCommandBuffers();
			void CreateCommandBuffers();

			bool CheckExtensionSupport(const char* p_Extension) const;
			bool CheckLayerSupport(const char* p_Layer) const;
			bool CheckDeviceExtensionSupport(const char* p_Extension) const;

			void DestroySwapchain();

		private:
			VkInstance m_Instance = VK_NULL_HANDLE;

		#ifdef YM_DEBUG
			VkDebugUtilsMessengerEXT m_Debugger = VK_NULL_HANDLE;
		#endif

			GLFWwindow* m_Window = nullptr;

			VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

			// Physical Device
			std::vector<PhysicalDevice> m_PhysicalDevices;
			uint32_t m_SelectedDeviceIndex = -1;
			uint32_t m_QueueFamily = -1;

			// Logical Device
			VkDevice m_Device = VK_NULL_HANDLE;
			VkQueue m_Queue = VK_NULL_HANDLE;

			// SwapChain
			VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
			uint32_t m_CurrentImageIndex = 0;
			uint32_t m_CurrentFrame = 0;
			uint32_t m_ImagesCount = 0;
			std::vector<VkImage> m_Images;
			std::vector<VkImageView> m_ImagesView;

			SwapchainOptions m_SwapchainOptions;
			ImagesViewOptions m_ImageViewsOptions;

			// RenderPass
			VkImageLayout m_RenderPassInitialImageLayout;
			VkImageLayout m_RenderPassFinalImageLayout;
			VkRenderPass m_RenderPass = VK_NULL_HANDLE;
			
			std::vector<VkFramebuffer> m_SwapchainFramebuffers;

			std::vector<VkSemaphore> m_SignalSemaphores;
			std::vector<VkSemaphore> m_WaitSemaphores;

			std::vector<VkFence> m_InFlightFences;

			bool m_ViewportResized = false;

			// Command Pool and Command Buffer
			VkCommandPool m_CommandPool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> m_CommandBuffers;

			bool m_HasDrawCommands = false;
	};
}