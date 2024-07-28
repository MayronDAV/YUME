#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/definitions.h"

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

		float LineWidth = 1.0f;
	};

	class YM_API Pipeline
	{
		friend class Shader;
		friend class VulkanShader;

		public:
			virtual ~Pipeline() = default;
			
			virtual Shader* GetShader() = 0;

			virtual void SetPolygonMode(PolygonMode p_Mode) = 0;

			static Ref<Pipeline> Create(const PipelineCreateInfo& p_CreateInfo);

		protected:
			virtual void Bind() = 0;
			PipelineCreateInfo m_CreateInfo;
	};
}