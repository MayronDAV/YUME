#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Core/definitions.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Core/command_buffer.h"
#include "renderpass.h"
#include "framebuffer.h"

#include <glm/glm.hpp>



namespace YUME
{
	class YM_API Shader;

	struct YM_API PipelineCreateInfo
	{
		Ref<Shader> Shader;

		CullMode	CullMode	   = CullMode::NONE;
		FrontFace	FrontFace	   = FrontFace::CLOCKWISE;
		PolygonMode PolygonMode	   = PolygonMode::FILL;
		DrawType	DrawType	   = DrawType::TRIANGLE;
		std::array<BlendMode, MAX_RENDER_TARGETS> BlendModes;

		bool TransparencyEnabled   = false;
		bool ClearTargets		   = true;
		bool DepthTest			   = true;
		bool DepthWrite			   = true;
		bool SwapchainTarget	   = true;
		bool DepthBiasEnabled	   = false;

		float LineWidth			   = 1.0f;
		float ConstantFactor	   = 0.0f;
		float SlopeFactor		   = 0.0f;

		float ClearColor[4]		   = {1, 1, 1, 1};
		std::array<Ref<Texture2D>, MAX_RENDER_TARGETS> ColorTargets;
		Ref<Texture2D> DepthTarget = nullptr;

		std::string DebugName	   = "Pipeline";
	};

	class YM_API Pipeline
	{
		friend class Shader;
		friend class VulkanShader;

		public:
			virtual ~Pipeline() = default;
			
			virtual Shader* GetShader() = 0;

			virtual void SetPolygonMode(PolygonMode p_Mode) = 0;

			virtual void Begin(CommandBuffer* p_CommandBuffer, SubpassContents p_Contents = SubpassContents::INLINE) = 0;
			virtual void End(CommandBuffer* p_CommandBuffer) = 0;

			uint32_t GetWidth();
			uint32_t GetHeight();

			virtual const Ref<RenderPass>& GetRenderPass() const = 0;
			virtual const Ref<Framebuffer>& GetFramebuffer() const = 0;

			static Ref<Pipeline> Create(const PipelineCreateInfo& p_CreateInfo);

			static Ref<Pipeline> Get(const PipelineCreateInfo& p_CreateInfo);
			static void ClearCache();
			static void DeleteUnusedCache();

		protected:
			PipelineCreateInfo m_CreateInfo;
	};
}