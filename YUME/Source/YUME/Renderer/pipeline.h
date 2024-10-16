#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Core/definitions.h"
#include "YUME/Renderer/texture.h"

#include <glm/glm.hpp>



namespace YUME
{
	class YM_API Shader;

	struct YM_API PipelineCreateInfo
	{
		Ref<Shader> Shader = nullptr; // Obrigatory

		CullMode CullMode = CullMode::NONE;
		FrontFace FrontFace = FrontFace::CLOCKWISE;
		PolygonMode PolygonMode = PolygonMode::FILL;
		DrawType DrawType = DrawType::TRIANGLE;
		BlendMode BlendMode = BlendMode::None;

		bool TransparencyEnabled = false;
		bool ClearTargets = false;
		bool DepthTest = true;
		bool DepthWrite = true;
		bool SwapchainTarget = false;

		float LineWidth = 1.0f;

		float ClearColor[4] = {1, 1, 1, 1};
		std::array<Ref<Texture2D>, MAX_RENDER_TARGETS> ColorTargets;
		Ref<Texture2D> DepthTarget = nullptr;
	};

	class YM_API Pipeline
	{
		friend class Shader;
		friend class VulkanShader;

		public:
			virtual ~Pipeline() = default;
			
			virtual Shader* GetShader() = 0;

			virtual void SetPolygonMode(PolygonMode p_Mode) = 0;

			virtual void Begin() = 0;
			virtual void End() = 0;

			virtual void OnResize(uint32_t p_Width, uint32_t p_Height) = 0;

			uint32_t GetWidth();
			uint32_t GetHeight();

			static Ref<Pipeline> Create(const PipelineCreateInfo& p_CreateInfo);

			static Ref<Pipeline> Get(const PipelineCreateInfo& p_CreateInfo);
			static void ClearCache();
			static void DeleteUnusedCache();

		protected:
			PipelineCreateInfo m_CreateInfo;
	};
}