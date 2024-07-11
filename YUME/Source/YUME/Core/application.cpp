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
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& p_Event)
	{
		EventDispatcher dispatcher(p_Event);
		dispatcher.Dispatch<WindowCloseEvent>(YM_BIND_EVENT_FN(Application::OnWindowClose));
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