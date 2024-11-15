#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Renderer/renderpass.h"



namespace YUME
{
	struct YM_API FramebufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		std::vector<Ref<Texture2D>> Attachments;
		Ref<RenderPass> RenderPass;
		std::string DebugName = "Framebuffer";
	};

	class YM_API Framebuffer
	{
		public:
			virtual ~Framebuffer() = default;

			virtual void OnResize(uint32_t p_Width, uint32_t p_Height, const std::vector<Ref<Texture2D>>& p_Attachments = {}) = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;

			static Ref<Framebuffer> Create(const FramebufferSpecification& p_Spec);

			static Ref<Framebuffer> Get(const FramebufferSpecification& p_Spec = {});
			static void ClearCache();
			static void DeleteUnusedCache();
	};
}