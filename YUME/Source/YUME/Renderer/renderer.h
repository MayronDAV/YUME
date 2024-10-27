#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Renderer/camera.h"

#include <glm/glm.hpp>



namespace YUME
{
	class YM_API Scene;

	struct YM_API RendererBeginInfo
	{
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		bool SwapchainTarget = true;
		Ref<Texture2D> RenderTarget = nullptr;
		uint32_t Width = 0;
		uint32_t Height = 0;
		Camera MainCamera;
		glm::mat4 CameraTransform{ 1.0f };
	};

	struct RenderSettings
	{
		bool DepthTest = false;
		bool Renderer2D = true;
	};

	class YM_API Renderer
	{
		public:
			static void Init();

			static void Begin(const RendererBeginInfo& p_BeginInfo);

			static void RenderScene(Scene* p_Scene);

			static void End();

			static RenderSettings& GetSettings();

			static void Shutdown();

		private:
			static void Render2DInit();
			static void Render2DFlush();
			static void Render2DStartBatch();
			static void Render2DFlushAndReset();


			static void FinalPass();
	};
}