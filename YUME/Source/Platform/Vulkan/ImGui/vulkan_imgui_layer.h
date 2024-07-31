#pragma once
#include "YUME/ImGui/imgui_layer.h"
#include "Platform/Vulkan/Core/vulkan_renderpass.h"
#include "Platform/Vulkan/Core/vulkan_scframebuffer.h"

// Lib
#include <vulkan/vulkan.h>


struct ImGui_ImplVulkanH_Window;

namespace YUME
{
	class YM_API VulkanImGuiLayer : public ImGuiLayer
	{
		public:
			VulkanImGuiLayer() = default;
			~VulkanImGuiLayer() override;
			
			void Recreate() override;

		protected:
			void Init() override;
			void NewFrame() override;
			void Render() override;
			void OnResize_Impl(uint32_t p_Width, uint32_t p_Height) override;
			void Clear() override;

		private:
			Ref<VulkanRenderPass> m_RenderPass = nullptr;
			
	};
}