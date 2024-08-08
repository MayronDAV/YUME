#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"

#include <string>
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>



struct Vertex
{
	glm::vec3 Position;
	glm::vec4 Color;
	glm::vec2 TexCoord;
};

struct Camera
{
	glm::mat4 Projection;
	glm::mat4 View;
};



class ExampleLayer : public YUME::Layer
{
	public:
		ExampleLayer()
			: Layer("Example")
		{
			m_Texture = YUME::TextureImporter::LoadTexture2D("Resources/statue.jpg");
			m_GrassTexture = YUME::TextureImporter::LoadTexture2D("Resources/green_grass.jpg");
		}

		void OnUpdate(YUME::Timestep p_Ts) override
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

			if (m_Wireframe)
			{
				YUME::Renderer2D::SetPolygonMode(YUME::PolygonMode::LINE);
			}
			else
			{
				YUME::Renderer2D::SetPolygonMode(YUME::PolygonMode::FILL);
			}

			if (m_UpdateColor)
			{
				m_Color.r = (float)std::cos(YUME::Clock::GetTime());
				m_Color.g = (float)std::cos(YUME::Clock::GetTime()) / (float)std::sin(YUME::Clock::GetTime());
				m_Color.b = (float)std::sin(YUME::Clock::GetTime());
			}
			YUME::RendererCommand::ClearColor(m_Color);

			YUME::Renderer2D::BeginScene();

			int size = 10;
			for (int x = -(size / 2); x < (size / 2); x++)
			{
				for (int y = -(size / 2); y < (size / 2); y++)
				{
					if ((std::abs(y) % 2) == 0)
					{
						YUME::Renderer2D::DrawQuad({ (float)x + ((float)x * 0.11f), (float)y + ((float)y * 0.11f), 0 }, { 1, 1 }, m_TileColor, m_GrassTexture);
					}
					else
					{
						YUME::Renderer2D::DrawQuad({ (float)x + ((float)x * 0.11f), (float)y + ((float)y * 0.11f), 0 }, { 1, 1 }, m_TileColor);
					}
				}
			}

			YUME::Renderer2D::DrawQuad(m_Position, {1, 1}, m_PlayerColor, m_Texture);

			YUME::Renderer2D::EndScene();
		}

		void OnImGuiRender() override
		{
			ImGui::Begin("Visualizer");
			ImGui::Text("FPS: %.0f", (float)YUME::Application::Get().GetFPS());
			ImGui::Text("Key Pressed: %s", m_CurrentKeyPressed.c_str());
			ImGui::End();

			ImGui::Begin("Editor");
			ImGui::DragFloat3("Player.Position", glm::value_ptr(m_Position), 1.0f);
			ImGui::ColorEdit4("Player.Color", glm::value_ptr(m_PlayerColor));
			ImGui::ColorEdit4("Tile Color", glm::value_ptr(m_TileColor));
			ImGui::ColorEdit4("Background", glm::value_ptr(m_Color));
			ImGui::End();

			ImGui::Begin("Render Stats");
			ImGui::Text("Quad Count: %d", YUME::Renderer2D::GetStats().QuadCount);
			ImGui::Text("Total Index Count: %d", YUME::Renderer2D::GetStats().GetTotalIndexCount());
			ImGui::Text("Total Vertex Count: %d", YUME::Renderer2D::GetStats().GetTotalVertexCount());
			ImGui::Text("Draw calls: %d", YUME::Renderer2D::GetStats().DrawCalls);
			ImGui::End();
		}

		void OnEvent(YUME::Event& p_Event) override
		{
			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::Space))
			{
				m_CurrentKeyPressed = "Space";
				m_UpdateColor = !m_UpdateColor;
			}

			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::J))
			{
				m_CurrentKeyPressed = "J";
				m_Wireframe = !m_Wireframe;
			}

			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::P))
			{
				m_CurrentKeyPressed = "P";
				YUME::Application::Get().ReloadImGui();
			}
		}

	private:
		glm::vec4 m_Color{0, 0, 0, 1};
		bool m_UpdateColor = false;

		bool m_Wireframe = false;

		glm::vec3 m_Position = { -0.5f, -0.5f, 0 };

		glm::vec4 m_PlayerColor = { 0.7f, 0.3f, 0.5f, 1.0f };
		glm::vec4 m_TileColor = { 0.5f, 0.3f, 0.7f, 1.0f };

		std::string m_CurrentKeyPressed = " ";

		YUME::Ref<YUME::Texture2D> m_Texture;
		YUME::Ref<YUME::Texture2D> m_GrassTexture;
		YUME::Ref<YUME::Texture2D> m_WhiteTexture;
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