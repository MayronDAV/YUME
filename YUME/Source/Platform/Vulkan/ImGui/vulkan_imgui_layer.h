#pragma once
#include "YUME/ImGui/imgui_layer.h"

// Lib
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>



namespace YUME
{
	class YM_API VulkanImGuiLayer : public ImGuiLayer
	{
		public:
			VulkanImGuiLayer();
			~VulkanImGuiLayer() override = default;

		protected:
			void Init() override;
			void NewFrame() override;
			void Render() override;
			void OnResize_Impl(uint32_t p_Width, uint32_t p_Height) override;
			void Clear() override;
		
	};
}