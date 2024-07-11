#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/window.h"


struct GLFWwindow;

namespace YUME::VULKAN
{
	class YM_PUBLIC Window : public YUME::Window
	{
		public:
			explicit Window(const WindowProps& props);
			~Window() override;

			void OnUpdate() override;

			uint32_t GetWidth() const override { return m_Data.Width; }
			uint32_t GetHeight() const override { return m_Data.Height; }

			// Window attributes
			void SetEventCallback(const EventCallbackFn& p_Callback) override { m_Data.EventCallback = p_Callback; }
			void SetVSync(bool p_Enabled) override;
			bool IsVSync() const override;

			void* GetNativeWindow() const override { return m_Window; }

			void SetCursorMode(CursorMode p_Mode) override;

		private:
			void Init(const WindowProps& p_Props);
			void Shutdown();

		private:
			GLFWwindow* m_Window;

			struct YM_LOCAL WindowData
			{
				std::string Title;
				uint32_t Width;
				uint32_t Height;
				bool VSync;

				EventCallbackFn EventCallback;
			};

			WindowData m_Data;
	};
}