#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/texture.h"


namespace YUME
{
	class YM_API RenderPassFramebuffer;

	struct YM_API RenderPassSpecification
	{
		std::vector<Ref<Texture2D>> Attachments;
		bool ClearEnable = true;
	};

	class YM_API RenderPass
	{
		public:
			virtual ~RenderPass() = default;

			virtual void Begin(const Ref<RenderPassFramebuffer>& p_Frame) = 0;
			virtual void End() = 0;

			virtual void SetClearColor(const glm::vec4& p_Color) = 0;
			virtual void EnableClearDepth(bool p_Enable) = 0;
			virtual void SetViewport(uint32_t p_Width, uint32_t p_Height) = 0;

			static Ref<RenderPass> Create(const RenderPassSpecification& p_Spec = {});
	};
}