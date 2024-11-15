#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Core/command_buffer.h"


namespace YUME
{
	class YM_API Framebuffer;

	struct YM_API RenderPassSpecification
	{
		std::vector<Ref<Texture2D>> Attachments;
		Ref<Texture2D> ResolveTexture = nullptr;
		int Samples = 1;
		bool ClearEnable = true;
		bool SwapchainTarget = false;
		std::string DebugName = "RenderPass";
	};

	class YM_API RenderPass
	{
		public:
			virtual ~RenderPass() = default;

			virtual void Begin(CommandBuffer* p_CommandBuffer, const Ref<Framebuffer>& p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color = { 1, 1, 1, 1 }, SubpassContents p_Contents = SubpassContents::INLINE) = 0;
			virtual void End(CommandBuffer* p_CommandBuffer) = 0;

			static Ref<RenderPass> Create(const RenderPassSpecification& p_Spec = {});

			static Ref<RenderPass> Get(const RenderPassSpecification& p_Spec = {});
			static void ClearCache();
			static void DeleteUnusedCache();
	};
}