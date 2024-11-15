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
	ImGuiContext* ImGuiLayer::GetCurrentContext()
	{
		return ImGui::GetCurrentContext();
	}

	uint32_t ImGuiLayer::GetActiveWidgetID() const
	{
		YM_CORE_ASSERT(GImGui)

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
