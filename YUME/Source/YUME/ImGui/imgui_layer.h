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
			void OnResize(uint32_t p_Width, uint32_t p_Height);

			void BlockEvents(bool p_Block) { m_BlockEvents = p_Block; }

			virtual void Begin() = 0;
			virtual void End()	 = 0;

			ImGuiContext* GetCurrentContext();

			virtual ImTextureID AddTexture(const Ref<Texture2D>& p_Texture) const = 0;

			uint32_t GetActiveWidgetID() const;

			static ImGuiLayer* Create();

		protected:
			bool  m_BlockEvents = true;
			void* m_DrawData	= nullptr;
	};
}