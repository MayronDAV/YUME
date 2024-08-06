#include "YUME/yumepch.h"
#include "YUME/Core/application.h"
#include "YUME/Core/log.h"

#include "YUME/Core/timestep.h"

#include "YUME/Renderer/renderer_command.h"
#include "YUME/Renderer/renderer3D.h"
#include "YUME/Utils/clock.h"

#include "YUME/Core/engine.h"

#include <iostream>
#include <imgui/imgui.h>




namespace YUME
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_ASSERT(s_Instance == nullptr)
		s_Instance = this;

		Engine::Init();

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(YM_BIND_EVENT_FN(Application::OnEvent));

		RendererCommand::Init(m_Window->GetContext());
		Renderer3D::Init();

		m_ImGuiLayer = ImGuiLayer::Create();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		YUME::RendererCommand::Shutdown();

		Engine::Release();

		YM_PROFILE_SHUTDOWN()
	}

	void Application::Run()
	{
		while (m_Running)
		{
			YM_PROFILE_FRAME("Application::MainLoop")

			double time = Clock::GetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (m_ReloadImGui)
			{
				YM_PROFILE_SCOPE("Appliation::Run - ReloadImGui")

				m_LayerStack.PopOverlay(m_ImGuiLayer);
				m_ImGuiLayer = ImGuiLayer::Create();
				PushOverlay(m_ImGuiLayer);

				ImGui::StyleColorsLight();

				m_ReloadImGui = false;
			}

			if (!m_Minimized)
			{
				if (double currentTime = Clock::GetTime();
					currentTime - m_LastTime >= 1.0f)
				{
					m_FPS = m_FPSCounter / (currentTime - m_LastTime);
					m_LastTime = currentTime;
					m_FPSCounter = 0;
				}

				m_Window->BeginFrame();

				for (auto& layer : m_LayerStack)
					layer->OnUpdate(timestep);

				m_ImGuiLayer->Begin();
				{
					for (Layer* layer : m_LayerStack)
						layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();

				//YM_CORE_INFO("FPS -> {}", (int)m_FPS)

				m_Window->EndFrame();

				m_FPSCounter++;
			}

			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* p_Layer)
	{
		m_LayerStack.PushLayer(p_Layer);
		p_Layer->OnAttach();
	}

	void Application::PushOverlay(Layer* p_Overlay)
	{
		m_LayerStack.PushOverlay(p_Overlay);
		p_Overlay->OnAttach();
	}

	void Application::OnEvent(Event& p_Event)
	{
		EventDispatcher dispatcher(p_Event);
		dispatcher.Dispatch<WindowCloseEvent>(YM_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(YM_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			//YM_CORE_TRACE("{0} -> {1}", (*it)->GetName(), p_Event.ToString())
			if (p_Event.Handled)
				break;
			(*it)->OnEvent(p_Event);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& p_Event)
	{
		m_Running = false;

		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& p_Event)
	{
		if (p_Event.GetWidth() == 0 || p_Event.GetHeight() == 0)
		{
			m_Minimized = true;
			return true;
		}

		m_Minimized = false;
		return true;
	}
}