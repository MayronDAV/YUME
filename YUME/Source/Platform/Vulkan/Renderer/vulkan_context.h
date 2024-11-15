#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/graphics_context.h"
#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"
#include "vulkan_renderpass.h"
#include "vulkan_framebuffer.h"
#include "YUME/Core/command_buffer.h"

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
			VulkanContext() = default;
			~VulkanContext() override;

			void Init(const char* p_Name, void* p_Window) override;

			void Begin() override;
			void End() override;

			void SwapBuffer() override;

			void OnResize(uint32_t p_Width, uint32_t p_Height) override;

			CommandBuffer* GetCurrentCommandBuffer() override;

			static void PushFunction(const std::function<void()>& p_Function)
			{
				m_MainDeletionQueue.PushFunction(p_Function);
			}

			static VkInstance GetInstance() { return s_Instance; }

		private:
			void CreateInstance(const char* p_Name);

			bool CheckExtensionSupport(const char* p_Extension) const;
			bool CheckLayerSupport(const char* p_Layer) const;

		private:
			static VkInstance s_Instance;

		#ifdef YM_DEBUG
			VkDebugUtilsMessengerEXT m_Debugger = VK_NULL_HANDLE;
		#endif

			GLFWwindow* m_Window = nullptr;

			static DeletionQueue m_MainDeletionQueue;
	};
}