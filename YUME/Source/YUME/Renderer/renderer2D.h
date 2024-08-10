#pragma once
#include "YUME/Core/base.h"
#include "YUME/Renderer/texture.h"
#include "camera.h"

// Lib
#include <glm/glm.hpp>



namespace YUME
{
	class YM_API Renderer2D
	{
		public:
			static void Init();
			static void Shutdown();

			static void BeginScene(const Camera& p_Camera, const glm::mat4& p_Transform);
			static void EndScene();

			static void SetPolygonMode(PolygonMode p_Mode);

			static void DrawQuad(const glm::vec3& p_Position, const glm::vec2& p_Size, const glm::vec4& p_Color = {1, 1, 1, 1}, const Ref<Texture2D>& p_Texture = nullptr);

			struct Statistics
			{
				uint32_t DrawCalls = 0;
				uint32_t QuadCount = 0;

				uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
				uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
			};
			static Statistics GetStats();
			static void ResetStats();

		private:
			static void Flush();
			static void StartBatch();
			static void FlushAndReset();
	};
}

