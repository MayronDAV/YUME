#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Renderer/buffer.h"

#include "YUME/Renderer/graphics_context.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Renderer/descriptor_set.h"
#include <glm/glm.hpp>


namespace YUME
{
	struct YM_API Capabilities
	{
		bool SupportTesselation;
		bool SupportGeometry;
		bool SupportCompute;
		bool SamplerAnisotropy;
		bool WideLines;
		bool FillModeNonSolid;
	};

	class YM_API RendererAPI
	{
		public:
			virtual ~RendererAPI() = default;

			virtual void Init(GraphicsContext* p_Context) = 0;

			virtual void SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height, CommandBuffer* p_CommandBuffer = nullptr) = 0;

			virtual void ClearColor(const glm::vec4& p_Color) = 0;
			virtual void ClearRenderTarget(const Ref<Texture2D>& p_Texture, uint32_t p_Value) = 0;
			virtual void ClearRenderTarget(const Ref<Texture2D>& p_Texture, const glm::vec4& p_Value) = 0;

			virtual void Draw(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, uint32_t p_VertexCount, uint32_t p_InstanceCount = 1) = 0;
			virtual void DrawIndexed(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, const Ref<IndexBuffer>& p_IndexBuffer, uint32_t p_InstanceCount = 1) = 0;

			virtual const Capabilities& GetCapabilities() const = 0;

			virtual void SaveScreenshot(const std::string& p_OutPath, const Ref<Texture>& p_Texture, bool p_Async = true) {};

			void BindDescriptorSets(CommandBuffer* p_CommandBuffer, const Ref<DescriptorSet>* p_DescriptorSets, uint32_t p_Count)
			{
				for (uint32_t i = 0; i < p_Count; i++)
				{
					p_DescriptorSets[i]->Bind(p_CommandBuffer);
				}
			}

			// Change this later
			static RendererAPI* Create();
	};
}