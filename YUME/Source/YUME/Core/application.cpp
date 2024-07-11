#include "YUME/yumepch.h"
#include "YUME/Core/application.h"
#include "YUME/Core/log.h"

#include <iostream>




namespace YUME
{
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(YM_BIND_EVENT_FN(Application::OnEvent));
	}

	void Application::Run()
	{
		while (m_Running)
		{
			if (!m_Minimized)
			{
				for (auto& layer : m_LayerStack)
					layer->OnUpdate();
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