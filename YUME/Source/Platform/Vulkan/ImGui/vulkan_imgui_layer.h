#pragma once
#include "YUME/ImGui/imgui_layer.h"
#include "Platform/Vulkan/Renderer/vulkan_renderpass.h"



namespace YUME
{
	class YM_API VulkanImGuiLayer : public ImGuiLayer
	{
		public:
			VulkanImGuiLayer() = default;
			~VulkanImGuiLayer() override;

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