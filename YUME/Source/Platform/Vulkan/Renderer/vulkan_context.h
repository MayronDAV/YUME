#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/graphics_context.h"
#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_renderpass_framebuffer.h"
#include "Platform/Vulkan/Core/vulkan_command_buffer.h"

#include "YUME/Utils/deletion_queue.h"

#include "YUME/Core/definitions.h"

// Lib
#include <vulkan/vulkan.h>



struct GLFWwindow;


namespace YUME
{
	class VulkanContext : public GraphicsContext
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

			// Draw
			void BeginFrame();
			void EndFrame();

			uint32_t GetCurrentFrame() const { return m_CurrentFrame;  }
			uint32_t GetCurrentImageIndex() const { return m_CurrentImageIndex; }

			VkSemaphore& GetSignalSemaphore() { return m_SignalSemaphores[m_CurrentFrame]; }
			VkSemaphore& GetWaitSemaphore() { return m_WaitSemaphores[m_CurrentFrame]; }

			VkFence& GetInFlightFence() { return m_InFlightFences[m_CurrentFrame]; }
			VkCommandBuffer& GetCommandBuffer() { return m_CommandBuffers.Get(m_CurrentFrame); }
			Ref<VulkanRenderPass>& GetRenderPass() { return m_RenderPass; }
			Ref<VulkanRenderPassFramebuffer> GetFramebuffer() { return m_Framebuffers[m_CurrentImageIndex]; }
			std::vector<Ref<VulkanRenderPassFramebuffer>>& GetFramebuffersList() { return m_Framebuffers; }

			static void PushFunction(const std::function<void()>& p_Function)
			{
				m_MainDeletionQueue.PushFunction(p_Function);
			}

			static void PushFunctionToFrameEnd(const std::function<void()>& p_Function)
			{
				m_FrameEndDeletionQueue.PushFunction(p_Function);
			}

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

			Ref<VulkanRenderPass> m_RenderPass;
			std::vector<Ref<VulkanRenderPassFramebuffer>> m_Framebuffers;

			std::vector<VkSemaphore> m_SignalSemaphores;
			std::vector<VkSemaphore> m_WaitSemaphores;
			std::vector<VkFence> m_InFlightFences;

			VulkanCommandBuffer m_CommandBuffers;

			bool m_ViewportResized = false;
			bool m_HasDrawCommands = false;

			static DeletionQueue m_MainDeletionQueue;
			static DeletionQueue m_FrameEndDeletionQueue;
	};
}