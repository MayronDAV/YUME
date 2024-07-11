#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"

#include <string>


class ExampleLayer : public YUME::Layer
{
	public:
		ExampleLayer()
			: Layer("Example") {}

		void OnUpdate() override
		{
			YM_TRACE("ExampleLayer::OnUpdate() -> {}", m_Runtime)
			m_Runtime++;
		}

		void OnEvent(YUME::Event& p_Event) override
		{
			YM_TRACE("ExampleLayer Event: {}", p_Event.ToString())
		}

	private:
		int m_Runtime = 0;
};


class Sandbox : public YUME::Application
{
	public:
		Sandbox()
		{
			PushLayer(new ExampleLayer());
		}

		~Sandbox() override = default;
};


YUME::Application* YUME::CreateApplication()
{
	return new Sandbox();
}