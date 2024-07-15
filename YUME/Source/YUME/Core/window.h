#pragma once

#include "YUME/yumepch.h"

#include "YUME/Core/base.h"
#include "YUME/Events/event.h"

#include "YUME/Renderer/graphics_context.h"



namespace YUME
{
	struct YM_PUBLIC WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "YUME Engine",
			uint32_t width = 800,
			uint32_t height = 600)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	enum class CursorMode
	{
		HIDDEN = 0, DISABLED, NORMAL
	};

	class YM_PUBLIC Window
	{
		public:
			using EventCallbackFn = std::function<void(Event&)>;

			virtual ~Window() = default;

			virtual void OnUpdate() = 0;

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;

			// Window attributes
			virtual void SetEventCallback(const EventCallbackFn& p_Callback) = 0;
			virtual void SetVSync(bool p_Enabled) = 0;
			virtual bool IsVSync() const = 0;

			virtual void* GetNativeWindow() const = 0;

			virtual void SetCursorMode(CursorMode p_Mode) = 0;

			virtual GraphicsContext* GetContext() = 0;

			static Window* Create(const WindowProps& p_Props = WindowProps());
	};

}
