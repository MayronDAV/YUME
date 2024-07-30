#include "YUME/yumepch.h"
#include "imgui_layer.h"
#include "YUME/Core/engine.h"

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
	}

	void ImGuiLayer::Begin()
	{
		NewFrame();
	}

	void ImGuiLayer::End()
	{
		Render();
	}

	void ImGuiLayer::OnResize(uint32_t p_Width, uint32_t p_Height)
	{
		OnResize_Impl(p_Width, p_Height);
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
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return new VulkanImGuiLayer();

		YM_CORE_VERIFY(false, "Unknown API!")
		return nullptr;
	}
}
