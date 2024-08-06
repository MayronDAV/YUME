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

	void RendererCommand::SetViewport(float p_X, float p_Y, uint32_t p_Width, uint32_t p_Height)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->SetViewport(p_X, p_Y, p_Width, p_Height);
	}
	void RendererCommand::Begin()
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->Begin();
	}
	void RendererCommand::End()
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->End();
	}
	void RendererCommand::ClearColor(const glm::vec4& p_Color)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->ClearColor(p_Color);
	}
	void RendererCommand::Draw(const Ref<VertexArray>& p_VertexArray, uint32_t p_VertexCount)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->Draw(p_VertexArray, p_VertexCount);
	}
	void RendererCommand::DrawIndexed(const Ref<VertexArray>& p_VertexArray, uint32_t p_IndexCount)
	{
		YM_PROFILE_FUNCTION()

		s_RendererAPI->DrawIndexed(p_VertexArray, p_IndexCount);
	}
}
