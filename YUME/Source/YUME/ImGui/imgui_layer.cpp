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
	}

	void ImGuiLayer::OnDetach()
	{
		Clear();
	}

	void ImGuiLayer::OnEvent(Event& p_Event)
	{
		if (m_BlockEvents)
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

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		//return GImGui->ActiveId;
		return 0;
	}

	ImGuiLayer* ImGuiLayer::Create()
	{
		if (Engine::GetAPI() == RenderAPI::Vulkan)
			return new VulkanImGuiLayer();

		YM_CORE_VERIFY(false, "Unknown API!")
		return nullptr;
	}
}
