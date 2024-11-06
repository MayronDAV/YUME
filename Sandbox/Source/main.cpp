#include "YUME/yume.h"
#include "YUME/Core/entry_point.h"
#include "editor_camera.h"


#include <string>
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_internal.h>
#include <random>




namespace YUME
{
	class ExampleLayer : public Layer
	{
	public:
		ExampleLayer()
			: Layer("Example")
		{
			m_Scene = new Scene();

			auto sphere = CreateRef<Model>("Resources/Meshes/sphere.obj");
			auto backpack = CreateRef<Model>("Resources/Meshes/backpack/backpack.obj", /* FlipYTexCoord */ false);
			auto artisansHub = CreateRef<Model>("Resources/Meshes/Spyro/ArtisansHub.obj");

			m_PointLight = m_Scene->CreateEntity("PointLight");
			m_PointLight.AddComponent<LightComponent>(LightType::Point, m_PointLightColor);
			m_PointLight.AddComponent<ModelComponent>(sphere);
			{
				auto& tc = m_PointLight.GetComponent<TransformComponent>();
				tc.Scale = glm::vec3(0.5f);
			}

			m_DirectionalLight = m_Scene->CreateEntity("DirectionalLight");
			m_DirectionalLight.AddComponent<LightComponent>(LightType::Directional, m_DirectionalLightColor);

			{
				auto entt = m_Scene->CreateEntity("Backpack");
				entt.AddComponent<ModelComponent>(backpack);
				auto& tc = entt.AddOrReplaceComponent<TransformComponent>(glm::vec3{ 5.0f, 3.0f, -4.0f });
			}

			{
				auto entt = m_Scene->CreateEntity("ArtisansHub");
				entt.AddComponent<ModelComponent>(artisansHub);
				auto& tc = entt.AddOrReplaceComponent<TransformComponent>(glm::vec3{ 5.0f, 0.0f, -4.0f });
			}
		}
		~ExampleLayer() { delete m_Scene; }


		void OnUpdate(const Timestep& p_Ts) override
		{
			m_Camera.SetViewportSize((float)m_Width, (float)m_Height);
			m_Camera.OnUpdate(p_Ts);

			{
				auto& tc = m_PointLight.GetComponent<TransformComponent>();
				tc.Translation = m_PointLightPosition;
				auto& lc = m_PointLight.GetComponent<LightComponent>();
				lc.Color = m_PointLightColor;
			}

			{
				auto& lc = m_DirectionalLight.GetComponent<LightComponent>();
				lc.Color = m_DirectionalLightColor;
				lc.Direction = glm::normalize(m_Direction);
			}

			RendererBeginInfo rbi{};
			rbi.MainCamera = m_Camera;
			rbi.ClearColor = m_Color;
			rbi.SwapchainTarget = false;
			rbi.Width = m_Width;
			rbi.Height = m_Height;

			Renderer::Begin(rbi);

			m_Scene->OnRender();

			Renderer::End();
		}

		void BeginDockSpace()
		{
			static bool opt_fullscreen = true;
			static bool opt_padding = false;
			static ImGuiDockNodeFlags DockspaceFlags = ImGuiDockNodeFlags_NoWindowMenuButton; /* ImGuiDockNodeFlags_None  */

			ImGuiID DockspaceID = ImGui::GetID("MyDockspace");

			ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				const ImGuiViewport* viewport = ImGui::GetMainViewport();

				ImGui::SetNextWindowPos(viewport->WorkPos);
				ImGui::SetNextWindowSize(viewport->WorkSize);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				WindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				WindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}
			else
			{
				DockspaceFlags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
			}

			if (DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
				WindowFlags |= ImGuiWindowFlags_NoBackground;

			if (!opt_padding)
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Editor Dockspace", nullptr, WindowFlags);
			if (!opt_padding)
				ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGui::DockSpace(DockspaceID, ImVec2(0.0f, 0.0f), DockspaceFlags);
			}
		}

		void EndDockSpace()
		{
			ImGui::End();
		}

		void OnImGuiRender() override
		{
			BeginDockSpace();

			ImGui::Begin("Visualizer");
			ImGui::Text("FPS: %.0f", (float)Application::Get().GetFPS());

			ImGui::Spacing();
			ImGui::Spacing();
			auto stats = Renderer::GetStats();

			ImGui::Text("Statistics: ");
			ImGui::Spacing();
			ImGui::Text("Quad Count: %i", stats.QuadCount);
			ImGui::Text("Circle Count: %i", stats.CircleCount);
			ImGui::Spacing();
			ImGui::Text("RenderScene Elapsed: %0.3f ms", stats.RenderSceneTimeMs);
			ImGui::Text("End Elapsed: %0.3f ms", stats.EndTimeMs);
			ImGui::Spacing();
			ImGui::Text("DrawCalls: %i", stats.DrawCalls);
			ImGui::End();

			ImGui::Begin("Editor");
			auto& position = m_Camera.GetPosition();
			ImGui::DragFloat3("Camera.Position", glm::value_ptr(position), 1.0f);
			ImGui::ColorEdit4("Background", glm::value_ptr(m_Color));
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::DragFloat3("PointLight.Position", glm::value_ptr(m_PointLightPosition), 0.1f);
			ImGui::ColorEdit3("PointLight.Color", glm::value_ptr(m_PointLightColor));
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::DragFloat3("DirectionalLight.Direction", glm::value_ptr(m_Direction), 0.1f, -1.0f, 1.0f);
			ImGui::ColorEdit3("DirectionalLight.Color", glm::value_ptr(m_DirectionalLightColor));
			ImGui::End();

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Begin("Scene View");
			{
				ImVec2 viewportSize = ImGui::GetContentRegionAvail();
				m_Width = (uint32_t)viewportSize.x;
				m_Height = (uint32_t)viewportSize.y;

				auto imgui = Application::Get().GetImGuiLayer();
				ImGui::Image(imgui->AddTexture(Renderer::GetRenderTexture()), { (float)m_Width, (float)m_Height });
			}
			ImGui::End();
			ImGui::PopStyleColor();

			EndDockSpace();
		}

		void OnEvent(Event& p_Event) override
		{
			m_Camera.OnEvent(p_Event);
		}

	private:
		glm::vec4 m_Color{ 0, 0, 0, 1 };
		EditorCamera m_Camera{ 90.0f, 1.778f, 0.1f, 1000.0f};

		Scene* m_Scene = nullptr;
		uint32_t m_Width = 800, m_Height = 600;

		glm::vec3 m_PointLightPosition{ 0.0f };
		glm::vec3 m_PointLightColor{ 1.0f };
		Entity m_PointLight;

		glm::vec3 m_DirectionalLightColor{ 1.0f };
		glm::vec3 m_Direction{ 1.0f, -1.0f, 0.0f };
		Entity m_DirectionalLight;
	};


	class Sandbox : public Application
	{
	public:
		Sandbox()
		{
			ImGui::SetCurrentContext(Application::Get(
			).GetImGuiLayer()->GetCurrentContext());

			PushLayer(new ExampleLayer());
		}

		~Sandbox() override = default;
	};


	Application* CreateApplication()
	{
		return new Sandbox();
	}
}