#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"

#include <string>
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>




class ExampleLayer : public YUME::Layer
{
	public:
		ExampleLayer()
			: Layer("Example")
		{
			m_Scene = new YUME::Scene();

			for (int x = 0; x < 10; x++)
			{
				for (int y = 0; y < 10; y++)
				{
					auto entt = m_Scene->CreateEntity("Grass");
					entt.AddComponent<YUME::SpriteComponent>(glm::vec4{1, 1, 1, 1}, "Resources/green_grass.jpg");
					entt.AddOrReplaceComponent<YUME::TransformComponent>(glm::vec3{x, y, 0.0f});
				}
			}
		}
		~ExampleLayer() { delete m_Scene; }


		void OnUpdate(const YUME::Timestep& p_Ts) override
		{
			float speed = 5.0f * (float)p_Ts;

			glm::vec3 direction(0.0f);
			if (YUME::Input::IsKeyPressed(YUME::Key::W))
			{
				m_CurrentKeyPressed = "W";
				direction.y = -1;
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::S))
			{
				m_CurrentKeyPressed = "S";
				direction.y = 1;
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::A))
			{
				m_CurrentKeyPressed = "A";
				direction.x = -1;
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::D))
			{
				m_CurrentKeyPressed = "D";
				direction.x = 1;
			}
			auto length = glm::length(direction);
			auto normalized = glm::normalize(direction);
			m_Position += ((length == 0) ? direction : normalized) * speed;


			uint32_t width = YUME::Application::Get().GetWindow().GetWidth();
			uint32_t height = YUME::Application::Get().GetWindow().GetHeight();
			float aspectRatio = (float)width / (float)height;
			auto projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
			auto transform = glm::translate(glm::mat4(1.0f), m_Position);

			YUME::RendererBeginInfo rbi{};
			rbi.MainCamera = YUME::Camera(projection);
			rbi.CameraTransform = transform;
			rbi.ClearColor = m_Color;
			rbi.SwapchainTarget = true;
			rbi.Width = width;
			rbi.Height = height;

			YUME::Renderer::Begin(rbi);

			m_Scene->OnRender();

			YUME::Renderer::End();
		}

		void OnImGuiRender() override
		{
			ImGui::Begin("Visualizer");
			ImGui::Text("FPS: %.0f", (float)YUME::Application::Get().GetFPS());
			ImGui::Text("Key Pressed: %s", m_CurrentKeyPressed.c_str());
			ImGui::End();

			ImGui::Begin("Editor");
			ImGui::DragFloat3("Player.Position", glm::value_ptr(m_Position), 1.0f);
			ImGui::ColorEdit4("Background", glm::value_ptr(m_Color));
			ImGui::End();
		}

		void OnEvent(YUME::Event& p_Event) override
		{
			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::P))
			{
				m_CurrentKeyPressed = "P";
				YUME::Application::Get().ReloadImGui();
			}
		}

	private:
		glm::vec4 m_Color{0, 0, 0, 1};
		glm::vec3 m_Position = { 0.0f, 0.0f, 10.0f };
		std::string m_CurrentKeyPressed = " ";

		YUME::Scene* m_Scene = nullptr;
};


class Sandbox : public YUME::Application
{
	public:
		Sandbox()
		{
			ImGui::SetCurrentContext(YUME::Application::Get().GetImGuiLayer()->GetCurrentContext());

			PushLayer(new ExampleLayer());
		}

		~Sandbox() override = default;
};


YUME::Application* YUME::CreateApplication()
{
	return new Sandbox();
}