#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"

#include <string>


class ExampleLayer : public YUME::Layer
{
	public:
		ExampleLayer()
			: Layer("Example") {}

		void OnUpdate(YUME::Timestep p_Ts) override 
		{
			glm::vec4 color(1, 1, 1, 1);
			color.r = std::cos(YUME::Clock::GetTime());
			color.g = std::cos(YUME::Clock::GetTime()) / std::sin(YUME::Clock::GetTime());
			color.b = std::sin(YUME::Clock::GetTime());

			YUME::RendererCommand::ClearColor(color);

			YUME::RendererCommand::Begin();

			YUME::RendererCommand::End();
		}

		void OnEvent(YUME::Event& p_Event) override
		{
			if (YUME::Input::IsKeyPressed(YUME::Key::W))
			{
				YM_CORE_INFO("W Is pressed!!!")
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::A))
			{
				YM_CORE_INFO("A Is pressed!!!")
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::S))
			{
				YM_CORE_INFO("S Is pressed!!!")
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::D))
			{
				YM_CORE_INFO("D Is pressed!!!")
			}
		}
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