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

struct Light
{
	glm::vec4 Color;
};

class ExampleLayer : public YUME::Layer
{
	public:
		ExampleLayer()
			: Layer("Example")
		{
			m_QuadShader = YUME::Shader::Create("assets/shaders/uniform_buffer_shader.glsl");

			YUME::PipelineCreateInfo pci{};
			pci.Shader = m_QuadShader;
			pci.BlendMode = YUME::BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			m_GraphicPipeline = YUME::Pipeline::Create(pci);

			m_QuadShader->SetPipeline(m_GraphicPipeline);

			m_VAO = YUME::VertexArray::Create();

			const std::vector<Vertex> vertices = {
				{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
				{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
				{{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
				{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
			};

			auto VBO = YUME::VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(Vertex));
			VBO->SetLayout({
				{ YUME::DataType::Float3, "a_Position"  },
				{ YUME::DataType::Float4, "a_Color"	    },
				{ YUME::DataType::Float2, "a_TexCoord"	}
			});
			m_VAO->AddVertexBuffer(VBO);

			std::vector<uint32_t> indexData = {
				0, 1, 2, 0, 2, 3
			};

			auto EBO = YUME::IndexBuffer::Create(indexData.data(), (uint32_t)indexData.size());
			m_VAO->SetIndexBuffer(EBO);

			m_QuadShader->AddVertexArray(m_VAO);

			m_CameraBuffer = YUME::UniformBuffer::Create(sizeof(Camera));
			m_LightBuffer = YUME::UniformBuffer::Create(sizeof(Light));

			m_Texture = YUME::TextureImporter::LoadTexture2D("Resources/statue.jpg");
		}

		void OnUpdate(YUME::Timestep p_Ts) override
		{
			float speed = 5.0f * (float)p_Ts;
			m_Position.z = 10.0f;

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
				m_GraphicPipeline->SetPolygonMode(YUME::PolygonMode::LINE);
			}
			else
			{
				m_GraphicPipeline->SetPolygonMode(YUME::PolygonMode::FILL);
			}

			if (m_UpdateColor)
			{
				m_Color.r = (float)std::cos(YUME::Clock::GetTime());
				m_Color.g = (float)std::cos(YUME::Clock::GetTime()) / (float)std::sin(YUME::Clock::GetTime());
				m_Color.b = (float)std::sin(YUME::Clock::GetTime());
			}
			YUME::RendererCommand::ClearColor(m_Color);

			YUME::RendererCommand::Begin();

			m_QuadShader->Bind();

			uint32_t width = YUME::Application::Get().GetWindow().GetWidth();
			uint32_t height = YUME::Application::Get().GetWindow().GetHeight();
			float aspectRatio = (float)width / (float)height;

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
			auto view = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
			Camera data = { projection, view };
			m_CameraBuffer->SetData(&data, sizeof(Camera));
			m_QuadShader->UploadUniformBuffer(m_CameraBuffer);

			//glm::vec4 color = { 0.1f, 0.1f, 0.1f, 1.0f };
			//m_LightBuffer->SetData(&color, sizeof(Light));
			//m_QuadShader->UploadUniformBuffer(m_LightBuffer);

			float size = 5.0f;
			for (float x = 0.0f; x < size; x++)
			{
				for (float y = 0.0f; y < size; y++)
				{
					m_QuadShader->PushFloat3("Player.Position", { x + (x * 0.11f), y + (y * 0.11f), 0.0f });
					m_QuadShader->PushFloat4("Player.Color", m_TileColor);
					YUME::RendererCommand::DrawIndexed(m_VAO, m_VAO->GetIndexCount());
				}
			}

			//m_QuadShader->UploadTexture2D(1, m_Texture);
			m_QuadShader->PushFloat3("Player.Position", { m_Position.x, m_Position.y, 0.0f });
			m_QuadShader->PushFloat4("Player.Color", m_PlayerColor);
			YUME::RendererCommand::DrawIndexed(m_VAO, m_VAO->GetIndexCount());

			YUME::RendererCommand::End();
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
		YUME::Ref<YUME::Pipeline> m_GraphicPipeline;
		YUME::Ref<YUME::Shader> m_QuadShader;
		YUME::Ref<YUME::VertexArray> m_VAO;
		YUME::Ref<YUME::UniformBuffer> m_CameraBuffer;
		YUME::Ref<YUME::UniformBuffer> m_LightBuffer;

		bool m_Wireframe = false;

		glm::vec3 m_Position = { 0.3f, 0.4f, 0.0f };
		glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
		glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };
		glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };

		glm::vec4 m_PlayerColor = { 0.7f, 0.3f, 0.5f, 1.0f };
		glm::vec4 m_TileColor = { 0.5f, 0.3f, 0.7f, 1.0f };

		std::string m_CurrentKeyPressed = " ";

		YUME::Ref<YUME::Texture2D> m_Texture;
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