#include "YUME/yumepch.h"
#include "YUME/Renderer/renderer_command.h"




namespace YUME
{
	RendererAPI* RendererCommand::s_RendererAPI = nullptr;


	void RendererCommand::Init(GraphicsContext* p_Context)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI = RendererAPI::Create();
		YM_CORE_VERIFY(s_RendererAPI, "have you defined the API?")
		s_RendererAPI->Init(p_Context);
	}

	void RendererCommand::Shutdown()
	{
		if (s_RendererAPI)
			delete s_RendererAPI;
	}

	void RendererCommand::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height, CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->SetViewport(p_X, p_Y, p_Width, p_Height, p_CommandBuffer);
	}

	void RendererCommand::ClearColor(const glm::vec4& p_Color)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->ClearColor(p_Color);
	}

	void RendererCommand::ClearRenderTarget(const Ref<Texture2D> p_Texture, uint32_t p_Value)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->ClearRenderTarget(p_Texture, p_Value);
	}

	void RendererCommand::ClearRenderTarget(const Ref<Texture2D> p_Texture, const glm::vec4& p_Value)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->ClearRenderTarget(p_Texture, p_Value);
	}

	void RendererCommand::Draw(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, uint32_t p_VertexCount, uint32_t p_InstanceCount)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->Draw(p_CommandBuffer, p_VertexBuffer, p_VertexCount, p_InstanceCount);
	}
	void RendererCommand::DrawIndexed(CommandBuffer* p_CommandBuffer, const Ref<VertexBuffer>& p_VertexBuffer, const Ref<IndexBuffer>& p_IndexBuffer, uint32_t p_InstanceCount)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->DrawIndexed(p_CommandBuffer, p_VertexBuffer, p_IndexBuffer, p_InstanceCount);
	}

	void RendererCommand::DrawMesh(CommandBuffer* p_CommandBuffer, const Ref<Mesh>& p_Mesh)
	{
		YM_PROFILE_FUNCTION()

		DrawIndexed(p_CommandBuffer, p_Mesh->GetVertexBuffer(), p_Mesh->GetIndexBuffer());
	}

	const Capabilities& RendererCommand::GetCapabilities()
	{
		YM_PROFILE_FUNCTION()

		return s_RendererAPI->GetCapabilities();
	}

	void RendererCommand::BindDescriptorSets(CommandBuffer* p_CommandBuffer, const Ref<DescriptorSet>* p_DescriptorSets, uint32_t p_Count)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->BindDescriptorSets(p_CommandBuffer, p_DescriptorSets, p_Count);
	}
}
