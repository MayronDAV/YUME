#include "YUME/yumepch.h"
#include "YUME/Renderer/renderer.h"
#include "YUME/Renderer/renderer2D.h"



namespace YUME
{
	void Renderer::Init()
	{
		YM_TRACE("Renderer Initialized!")

		Renderer2D::Init();
	}

	void Renderer::Shutdown()
	{
		Renderer2D::Shutdown();
	}
}