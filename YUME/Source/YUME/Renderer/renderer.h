#pragma once

#include "YUME/Core/base.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Renderer/camera.h"

#include <glm/glm.hpp>



namespace YUME
{
	class YM_API Scene;

	enum class Quality
	{
		Low = 0,
		Medium,
		High
	};

	// TODO: Move to project settings when we have projects
	struct RenderSettings
	{
		Quality ShadowMap		= Quality::Medium;
		bool PBR				= true;
		bool OIT				= false;
		bool Skybox				= true;
		bool Renderer3D			= true;
		bool Renderer2D			= false;
		bool Renderer2D_Quad	= true;
		bool Renderer2D_Circle  = true;
	};

	struct YM_API RendererBeginInfo
	{
		bool	  SwapchainTarget = false;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		uint32_t  Width = 0;
		uint32_t  Height = 0;
		Camera    MainCamera;
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

			static void OnImgui();

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