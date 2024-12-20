#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/window.h"

#include "Platform/Vulkan/Renderer/vulkan_context.h"




struct GLFWwindow;

namespace YUME
{
	class VulkanWindow : public Window
	{
		public:
			explicit VulkanWindow(const WindowProps& props);
			~VulkanWindow() override;

			void BeginFrame() override;
			void EndFrame() override;

			void OnUpdate() override;

			uint32_t GetWidth() const override { return m_Data.Width; }
			uint32_t GetHeight() const override { return m_Data.Height; }

			// Window attributes
			void SetEventCallback(const EventCallbackFn& p_Callback) override { m_Data.EventCallback = p_Callback; }
			void SetVSync(bool p_Enabled) override;
			bool IsVSync() const override;

			void* GetNativeWindow() const override { return m_Window; }
			const std::vector<Resolution>& GetResolutions() override { return m_Resolutions; }

			GraphicsContext* GetContext() override;

			void SetCursorMode(CursorMode p_Mode) override;

		private:
			void Init(const WindowProps& p_Props);
			void Shutdown();

		private:
			GLFWwindow* m_Window;

			struct WindowData
			{
				std::string Title;
				uint32_t Width;
				uint32_t Height;
				bool Vsync;
				Unique<VulkanContext> Context;

				EventCallbackFn EventCallback;
			};
			WindowData m_Data;

			std::vector<Resolution> m_Resolutions;
	};
}