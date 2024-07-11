#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/window.h"

#include "YUME/Events/event.h"
#include "YUME/Events/application_event.h"
#include "YUME/Core/layer_stack.h"


namespace YUME
{
	class YM_PUBLIC Application
	{
		public:
			Application();
			virtual ~Application() = default;

			void Run();

			void PushLayer(Layer* p_Layer);
			void PushOverlay(Layer* p_Overlay);

			void OnEvent(Event& p_Event);

		private:
			bool OnWindowClose(WindowCloseEvent& p_Event);
			bool OnWindowResize(WindowResizeEvent& p_Event);

		private:
			std::unique_ptr<Window> m_Window;
			bool m_Running = true;
			bool m_Minimized = false;

			LayerStack m_LayerStack;
	};

	Application* CreateApplication();
}

