#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/layer.h"
#include "YUME/Renderer/texture.h"


struct ImGuiContext;
typedef void* ImTextureID;

namespace YUME
{
	class YM_API ImGuiLayer : public Layer
	{
		public:
			ImGuiLayer() : Layer("ImGuiLayer") {}
			~ImGuiLayer() override = default;

			// Layer

			void OnAttach() override;
			void OnDetach() override;
			void OnEvent(Event& p_Event) override;

			// Imgui

			void OnResize(uint32_t p_Width, uint32_t p_Height);

			void BlockEvents(bool p_Block) { m_BlockEvents = p_Block; }

			void Begin();
			void End();

			ImGuiContext* GetCurrentContext();

			virtual ImTextureID AddTexture(const Ref<Texture2D>& p_Texture) const = 0;

			uint32_t GetActiveWidgetID() const;

			static ImGuiLayer* Create();

		protected:
			virtual void Init() = 0;
			virtual void NewFrame() = 0;
			virtual void Render() = 0;
			virtual void OnResize_Impl(uint32_t p_Width, uint32_t p_Height) = 0;
			virtual void Clear() = 0;

		protected:
			bool m_BlockEvents = true;
			bool m_IsAttached = false;
			void* m_DrawData = nullptr;
	};
}