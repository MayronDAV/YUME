#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/graphics_context.h"
#include "Platform/Vulkan/Core/vulkan_swapchain.h"
#include "Platform/Vulkan/Core/vulkan_renderpass.h"
#include "Platform/Vulkan/Core/vulkan_scframebuffer.h"
#include "Platform/Vulkan/Core/vulkan_command_buffer.h"

#include "YUME/Utils/deletion_queue.h"

#include "YUME/Core/definitions.h"

// Lib
#include <vulkan/vulkan.h>



struct GLFWwindow;


namespace YUME
{
	class YM_API VulkanContext : public GraphicsContext
	{
		public:
			~VulkanContext() override;

			void Init(void* p_Window) override;

			void SwapBuffer() override;

			void RecreateSwapchain();

			void OnResize() { m_ViewportResized = true; }


			// Sync Objs
			void WaitFence();
			void ResetFence();


			void HasDrawCommands(bool p_Has) { m_HasDrawCommands = p_Has; }

			void TransitionImageLayout(VkImage p_Image, VkFormat p_Format, VkImageLayout p_OldLayout, VkImageLayout p_NewLayout) const;

			// Draw
			void BeginFrame();

			uint32_t GetCurrentFrame() const { return m_CurrentFrame;  }

			VkSemaphore& GetSignalSemaphore() { return m_SignalSemaphores[m_CurrentFrame]; }
			VkSemaphore& GetWaitSemaphore() { return m_WaitSemaphores[m_CurrentFrame]; }

			VkFence& GetInFlightFence() { return m_InFlightFences[m_CurrentFrame]; }
			VulkanCommandBuffer& GetCommandBuffer() { return m_CommandBuffers; }
			Ref<VulkanRenderPass>& GetRenderPass() { return m_RenderPass; }
			VulkanSCFramebuffer& GetSwapchainFramebuffer() { return m_SCFramebuffers[m_CurrentImageIndex]; }


			static VkInstance GetInstance() { return s_Instance; }

		private:
			void CreateInstance();
			void CreateSyncObjs();

			bool CheckExtensionSupport(const char* p_Extension) const;
			bool CheckLayerSupport(const char* p_Layer) const;

		private:
			static VkInstance s_Instance;

		#ifdef YM_DEBUG
			VkDebugUtilsMessengerEXT m_Debugger = VK_NULL_HANDLE;
		#endif

			GLFWwindow* m_Window = nullptr;

			uint32_t m_CurrentImageIndex = 0;
			uint32_t m_CurrentFrame = 0;
			uint32_t m_ImagesCount = 0;

			// RenderPass
			Ref<VulkanRenderPass> m_RenderPass;
			
			// Swapchain framebuffers
			std::vector<VulkanSCFramebuffer> m_SCFramebuffers;

			std::vector<VkSemaphore> m_SignalSemaphores;
			std::vector<VkSemaphore> m_WaitSemaphores;
			std::vector<VkFence> m_InFlightFences;

			VulkanCommandBuffer m_CommandBuffers;

			bool m_ViewportResized = false;
			bool m_HasDrawCommands = false;
	};
}