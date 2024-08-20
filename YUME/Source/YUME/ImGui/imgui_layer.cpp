#include "YUME/yumepch.h"
#include "imgui_layer.h"
#include "YUME/Core/engine.h"
#include <YUME/Events/application_event.h>
#include "Platform/Vulkan/ImGui/vulkan_imgui_layer.h"

// Lib
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>




namespace YUME
{

	void ImGuiLayer::OnAttach()
	{
		Init();
		m_IsAttached = true;
	}

	void ImGuiLayer::OnDetach()
	{
		Clear();
		m_IsAttached = false;
	}

	void ImGuiLayer::OnEvent(Event& p_Event)
	{
		if (m_BlockEvents && m_IsAttached)
		{
			const ImGuiIO& io = ImGui::GetIO();
			p_Event.Handled |= p_Event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			p_Event.Handled |= p_Event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}

		if (p_Event.GetEventType() == WindowResizeEvent::GetStaticType())
		{
			auto event = (WindowResizeEvent*)&p_Event;
			YM_CORE_INFO("ImGui resize handled!")
			OnResize_Impl(event->GetWidth(), event->GetHeight());
		}
	}

	void ImGuiLayer::OnResize(uint32_t p_Width, uint32_t p_Height)
	{
		OnResize_Impl(p_Width, p_Height);
	}

	void ImGuiLayer::Begin()
	{
		NewFrame();
	}

	void ImGuiLayer::End()
	{
		Render();
	}

	ImGuiContext* ImGuiLayer::GetCurrentContext()
	{
		return ImGui::GetCurrentContext();
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		YM_CORE_ASSERT(m_IsAttached)

		return GImGui->ActiveId;
	}

	ImGuiLayer* ImGuiLayer::Create()
	{
		YM_PROFILE_FUNCTION()

		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return new VulkanImGuiLayer();

		YM_CORE_VERIFY(false, "Unknown API!")
		return nullptr;
	}
}
