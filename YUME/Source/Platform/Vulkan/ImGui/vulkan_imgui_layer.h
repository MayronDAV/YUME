#pragma once
#include "YUME/ImGui/imgui_layer.h"
#include "Platform/Vulkan/Renderer/vulkan_renderpass.h"
#include "Platform/Vulkan/Renderer/vulkan_framebuffer.h"



namespace YUME
{
	class VulkanImGuiLayer : public ImGuiLayer
	{
		public:
			VulkanImGuiLayer() = default;
			~VulkanImGuiLayer() override;

			void OnAttach() override;
			void OnDetach() override;
			void OnEvent(Event& p_Event) override;

			void Begin() override;
			void End() override;

			ImTextureID AddTexture(const Ref<Texture2D>& p_Texture) const override;

		private:
			Ref<VulkanRenderPass> m_RenderPass = nullptr;
			std::vector<Ref<VulkanFramebuffer>> m_Framebuffers;
	};
}