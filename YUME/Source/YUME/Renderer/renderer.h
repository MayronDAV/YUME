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
		uint32_t Width = 0;
		uint32_t Height = 0;
		Camera MainCamera;
		glm::mat4 CameraTransform{ 1.0f };
	};

	// Maybe move to application or project settings?
	struct RenderSettings
	{
		bool PBR = true;
		bool OIT = false;
		bool Renderer3D = true;
		bool Renderer2D = false;
		bool Renderer2D_Quad = true;
		bool Renderer2D_Circle = true;
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

			static Ref<Texture2D> GetRenderTexture();

			static void SetPolygonMode(PolygonMode p_Mode);

			struct Statistics
			{
				uint32_t QuadCount = 0;
				uint32_t CircleCount = 0;

				double RenderSceneTimeMs = 0.0;
				double EndTimeMs = 0.0;

				uint32_t DrawCalls = 0;
			};
			static Statistics GetStats();

		private:
			static void ResetStats();

			static void FinalPass();
	};
}