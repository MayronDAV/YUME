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
			m_Texture = YUME::TextureImporter::LoadTexture2D("Resources/statue.jpg");
			m_NokotanTexture = YUME::TextureImporter::LoadTexture2D("Resources/nokotan.jpg");
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

			uint32_t width = YUME::Application::Get().GetWindow().GetWidth();
			uint32_t height = YUME::Application::Get().GetWindow().GetHeight();
			float aspectRatio = (float)width / (float)height;
			auto projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
			auto transform = glm::translate(glm::mat4(1.0f), m_Position);

			YUME::Renderer2D::BeginScene(YUME::Camera(projection), transform);

			for (int x = -(m_TileCount / 2); x < (m_TileCount / 2); x++)
			{
				for (int y = -(m_TileCount / 2); y < (m_TileCount / 2); y++)
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

			YUME::Renderer2D::DrawQuad({ -(m_TileCount / 2) - 2, -(m_TileCount / 2) - 2, m_Nokotan_z }, { 1, 1 }, m_TileColor, m_NokotanTexture);

			auto playerPos = m_Position;
			playerPos.z = 0.0f;
			YUME::Renderer2D::DrawQuad(playerPos, {1, 1}, m_PlayerColor, m_Texture);

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
			ImGui::InputInt("Tile Count", &m_TileCount);
			ImGui::InputFloat("Nokotan Z", &m_Nokotan_z);
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

		glm::vec3 m_Position = { 0.0f, 0.0f, 10.0f };
		float m_Nokotan_z = 0.0f;

		glm::vec4 m_PlayerColor = { 0.7f, 0.3f, 0.5f, 1.0f };
		glm::vec4 m_TileColor = { 0.5f, 0.3f, 0.7f, 1.0f };

		std::string m_CurrentKeyPressed = " ";

		YUME::Ref<YUME::Texture2D> m_Texture;
		YUME::Ref<YUME::Texture2D> m_NokotanTexture;
		YUME::Ref<YUME::Texture2D> m_GrassTexture;
		YUME::Ref<YUME::Texture2D> m_WhiteTexture;

		int m_TileCount = 11;
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