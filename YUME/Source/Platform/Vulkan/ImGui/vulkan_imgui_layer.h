#pragma once
#include "YUME/ImGui/imgui_layer.h"
#include "Platform/Vulkan/Renderer/vulkan_renderpass.h"
#include "Platform/Vulkan/Renderer/vulkan_renderpass_framebuffer.h"



namespace YUME
{
	class VulkanImGuiLayer : public ImGuiLayer
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
			std::vector<Ref<VulkanRenderPassFramebuffer>> m_Framebuffers;
	};
}