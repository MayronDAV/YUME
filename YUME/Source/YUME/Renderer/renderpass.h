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
		Ref<Texture2D> ResolveTexture = nullptr;
		int Samples = 1;
		bool ClearEnable = true;
		bool SwapchainTarget = false;
	};

	class YM_API RenderPass
	{
		public:
			virtual ~RenderPass() = default;

			virtual void Begin(const Ref<RenderPassFramebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color = { 1, 1, 1, 1 }) = 0;
			virtual void End() = 0;

			static Ref<RenderPass> Create(const RenderPassSpecification& p_Spec = {});

			static Ref<RenderPass> Get(const RenderPassSpecification& p_Spec = {});
			static void ClearCache();
			static void DeleteUnusedCache();
	};
}