#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"

#include <string>
#include <imgui/imgui.h>



struct Vertice
{
	glm::vec3 Position;
	glm::vec4 Color;
};


class ExampleLayer : public YUME::Layer
{
	public:
		ExampleLayer()
			: Layer("Example")
		{
			m_QuadShader = YUME::Shader::Create("assets/shaders/quad_shader.glsl");

			YUME::PipelineCreateInfo pci{};
			pci.Shader = m_QuadShader;
			m_GraphicPipeline = YUME::Pipeline::Create(pci);

			m_QuadShader->SetPipeline(m_GraphicPipeline);

			m_VAO = YUME::VertexArray::Create();

			std::vector<Vertice> inputData = {
				Vertice{{-0.5f,-0.5f, 0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f }},
				Vertice{{ 0.5f,-0.5f, 0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
				Vertice{{ 0.5f, 0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f, 1.0f }},
				Vertice{{-0.5f, 0.5f, 0.0f }, { 1.0f, 0.0f, 1.0f, 1.0f }},
			};

			YUME::Ref<YUME::VertexBuffer> VBO = YUME::VertexBuffer::Create(inputData.size() * sizeof(Vertice));
			VBO->SetLayout({
				{ YUME::DataType::Float3, "a_Position"  },
				{ YUME::DataType::Float4, "a_Color"	    }
			});
			VBO->SetData(inputData.data(), inputData.size() * sizeof(Vertice));
			m_VAO->AddVertexBuffer(VBO);

			std::vector<uint32_t> indexData = {
				0, 1, 2, 0, 2, 3
			};

			YUME::Ref<YUME::IndexBuffer> quadIB = YUME::IndexBuffer::Create(indexData.data(), (uint32_t)indexData.size());
			m_VAO->SetIndexBuffer(quadIB);

			m_QuadShader->AddVertexArray(m_VAO);
		}

		void OnUpdate(YUME::Timestep p_Ts) override
		{
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

			YUME::RendererCommand::DrawIndexed(m_VAO, m_VAO->GetIndexCount());

			YUME::RendererCommand::End();
		}

		void OnImGuiRender() override
		{
			ImGui::SetCurrentContext(YUME::Application::Get().GetImGuiLayer()->GetCurrentContext());

			ImGui::Begin("Perfomance visualizer");
			ImGui::Text("FPS: %.0f", (float)YUME::Application::Get().GetFPS());
			ImGui::End();
		}

		void OnEvent(YUME::Event& p_Event) override
		{
			if (YUME::Input::IsKeyPressed(YUME::Key::W))
			{
				YM_CORE_WARN("W Is pressed!!!")
				m_Color = { 1, 0, 0, 1 };
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::A))
			{
				YM_CORE_WARN("A Is pressed!!!")
				m_Color = { 0, 1, 0, 1 };
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::S))
			{
				YM_CORE_WARN("S Is pressed!!!")
				m_Color = { 0, 0, 1, 1 };
			}
			if (YUME::Input::IsKeyPressed(YUME::Key::D))
			{
				YM_CORE_WARN("D Is pressed!!!")
				m_Color = {1, 1, 1, 1};
			}

			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::Space))
			{
				YM_CORE_WARN("Space Is pressed!!!")
				m_UpdateColor = !m_UpdateColor;
			}

			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::J))
			{
				YM_CORE_WARN("J Is pressed!!!")
				m_Wireframe = !m_Wireframe;
			}

			if (YUME::Input::IsKeyPressedOnce(p_Event, YUME::Key::P))
			{
				YM_CORE_WARN("P Is pressed!!!")
				YUME::Application::Get().ReloadImGui();
			}
		}

	private:
		glm::vec4 m_Color{1, 1, 1, 1};
		bool m_UpdateColor = true;
		YUME::Ref<YUME::Pipeline> m_GraphicPipeline;
		YUME::Ref<YUME::Shader> m_QuadShader;

		YUME::Ref<YUME::VertexArray> m_VAO;

		bool m_Wireframe = false;
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