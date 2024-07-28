#pragma once
#include "definitions.h"


namespace YUME
{
	// In the future, this class will handle all engine configurations
	// including config files.

	class Engine
	{
		public:
			static void Init();
			static void Release();

			static RenderAPI GetAPI() { return s_Instance->m_API; }
			static void SetAPI(RenderAPI p_API) { s_Instance->m_API = p_API; }

		private:
			Engine() = default;

		private:
			static Engine* s_Instance;
			RenderAPI m_API = RenderAPI::Vulkan;
	};
}