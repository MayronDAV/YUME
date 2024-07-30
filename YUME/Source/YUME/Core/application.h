#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/window.h"

#include "YUME/Events/event.h"
#include "YUME/Events/application_event.h"
#include "YUME/Core/layer_stack.h"

#include "YUME/ImGui/imgui_layer.h"


namespace YUME
{
	class YM_API Application
	{
		public:
			Application();
			virtual ~Application();

			void Run();

			void PushLayer(Layer* p_Layer);
			void PushOverlay(Layer* p_Overlay);

			void OnEvent(Event& p_Event);

			double GetFPS() const { return m_FPS; }

			Window& GetWindow() { return *m_Window.get(); }

			inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

			void ReloadImGui() { m_ReloadImGui = true; }

			static Application& Get() { return *s_Instance; }

		private:
			bool OnWindowClose(WindowCloseEvent& p_Event);
			bool OnWindowResize(WindowResizeEvent& p_Event);

		private:
			std::unique_ptr<Window> m_Window;
			bool m_Running = true;
			bool m_Minimized = false;
			double m_LastFrameTime = 0.0;

			LayerStack m_LayerStack;

			double m_LastTime = 0;
			double m_FPSCounter = 0;
			double m_FPS = 0;

			ImGuiLayer* m_ImGuiLayer = nullptr;
			bool m_ReloadImGui = false;


		private:
			static Application* s_Instance;
	};

	Application* CreateApplication();
}

