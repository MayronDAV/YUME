#include "YUME/yumepch.h"
#include "YUME/Renderer/renderer.h"
#include "YUME/Scene/scene.h"
#include "YUME/Scene/entity.h"
#include "YUME/Scene/entity_manager.h"
#include "shader.h"
#include "descriptor_set.h"
#include "renderer_command.h"


#include <glm/gtc/type_ptr.hpp>
#include <Platform/Vulkan/Renderer/vulkan_texture.h>
#include "Platform/Vulkan/Utils/vulkan_utils.h"




namespace YUME
{

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		int TexIndex = 0;
	};

	struct Render2Ddata
	{
		static const uint32_t MaxQuads = 5000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint8_t MaxTextureSlots = 32;

		bool First = true;

		// ----------------------------
		// QUAD stuff

		Ref<VertexArray> QuadVertexArray = nullptr;
		Ref<VertexBuffer> QuadVertexBuffer = nullptr;
		Ref<Shader> QuadShader = nullptr;
		Ref<Pipeline> QuadPipeline = nullptr;
		Ref<DescriptorSet> QuadDescriptorSet = nullptr;

		uint32_t QuadIndexCount = 0;
		std::vector<QuadVertex> QuadVertexBufferBase;

		std::array<glm::vec4, 4> QuadVertexPositions;

		// ----------------------------
		// OTHER stuff

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer{ glm::mat4(1.0f) };
		Ref<UniformBuffer> CameraUniformBuffer = nullptr;

		// ----------------------------
	};

	struct RenderData
	{
		bool CalledBegin = false;
		glm::vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		Ref<Texture2D> MainTexture = nullptr;
		Ref<Texture2D> WhiteTexture = nullptr;
		bool SwapchainTarget = true; 
		Ref<Texture2D> RenderTarget = nullptr;
		uint32_t Width;
		uint32_t Height;

		Ref<Shader> FinalPassShader = nullptr;
		Ref<Pipeline> FinalPassPipeline = nullptr;
		Ref<DescriptorSet> FinalPassDescriptorSet = nullptr;

		RenderSettings Settings = {};
	};

	static RenderData* s_RenderData;
	static Render2Ddata* s_Render2Ddata;

	void Renderer::Init()
	{
		YM_PROFILE_FUNCTION()

		YM_TRACE("Renderer Initialized!")

		s_RenderData = new RenderData();
		s_Render2Ddata = new Render2Ddata();

		s_RenderData->FinalPassShader = Shader::Create("assets/shaders/FinalPassShader.glsl");

		s_RenderData->FinalPassDescriptorSet = DescriptorSet::Create(s_RenderData->FinalPassShader);

		uint8_t whiteData[] = { 255, 255, 255, 255 };
		s_RenderData->WhiteTexture = Texture2D::Create({}, whiteData, sizeof(whiteData));
	}

	void Renderer::Begin(const RendererBeginInfo& p_BeginInfo)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(!s_RenderData->CalledBegin, "Did you call End()?")
		YM_CORE_VERIFY(p_BeginInfo.RenderTarget || p_BeginInfo.SwapchainTarget)
		YM_CORE_VERIFY(p_BeginInfo.Width != 0 && p_BeginInfo.Height != 0)

		s_RenderData->CalledBegin = true;
		s_RenderData->ClearColor = p_BeginInfo.ClearColor;
		s_RenderData->SwapchainTarget = p_BeginInfo.SwapchainTarget;
		s_RenderData->RenderTarget = p_BeginInfo.RenderTarget;
		s_RenderData->Width = p_BeginInfo.Width;
		s_RenderData->Height = p_BeginInfo.Height;

		TextureSpecification texSpec{};
		texSpec.Width = s_RenderData->Width;
		texSpec.Height = s_RenderData->Height;
		texSpec.Format = TextureFormat::RGBA8_SRGB;
		texSpec.Usage = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		s_RenderData->MainTexture = Texture2D::Get(texSpec);

		RendererCommand::ClearRenderTarget(s_RenderData->MainTexture, s_RenderData->ClearColor);

		if (s_RenderData->Settings.Renderer2D)
		{
			if (s_Render2Ddata->First)
			{
				Render2DInit();
				s_Render2Ddata->First = false;
			}

			PipelineCreateInfo pci{};
			pci.Shader = s_Render2Ddata->QuadShader;
			pci.BlendMode = BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			pci.ColorTargets[0] = s_RenderData->MainTexture;
			pci.ClearTargets = false;
			pci.SwapchainTarget = false;
			pci.DepthTest = false;
			pci.DepthWrite = false;
			std::memcpy(pci.ClearColor, glm::value_ptr(s_RenderData->ClearColor), 4 * sizeof(float));
			s_Render2Ddata->QuadPipeline = Pipeline::Get(pci);

			s_Render2Ddata->CameraBuffer.ViewProjection = p_BeginInfo.MainCamera.GetProjection() * glm::inverse(p_BeginInfo.CameraTransform);
			s_Render2Ddata->CameraUniformBuffer->SetData(&s_Render2Ddata->CameraBuffer, sizeof(Render2Ddata::CameraData));

			Render2DStartBatch();
		}

	}

	void Renderer::RenderScene(Scene* p_Scene)
	{
		YM_PROFILE_FUNCTION()

		auto enttManager = p_Scene->GetEntityManager();
		auto& registry = p_Scene->GetRegistry();

		if (s_RenderData->Settings.Renderer2D)
		{
			auto group = enttManager->GetEntitiesWithTypes<SpriteComponent>();
			for (auto entt : group)
			{
				Entity entity = { entt, p_Scene };
				auto& sprite = entity.GetComponent<SpriteComponent>();
				auto& shape = entity.GetComponent<ShapeComponent>();
				auto& tc = entity.GetComponent<TransformComponent>();

				if (shape.IsShape<Square>())
				{
					YM_PROFILE_SCOPE("Batching quad")

					size_t quadVertexCount = 4;
					const glm::vec2* texcoord;
					if (sprite.Texture != nullptr)
						texcoord = sprite.Texture->GetTexCoords();
					else
					{
						texcoord = new glm::vec2[quadVertexCount]
						{
							{ 0.0f, 0.0f },
							{ 1.0f, 0.0f },
							{ 1.0f, 1.0f },
							{ 0.0f, 1.0f },
						};
					}

					if (s_Render2Ddata->QuadIndexCount >= Render2Ddata::MaxIndices)
						Render2DFlushAndReset();

					int textureIndex = 0; // White texture
					if (sprite.Texture != nullptr)
					{
						for (uint32_t i = 1; i < s_Render2Ddata->TextureSlotIndex; i++)
						{
							if (*s_Render2Ddata->TextureSlots[i].get() == *sprite.Texture->GetTexture().get())
							{
								textureIndex = i;
								break;
							}
						}

						if (textureIndex == 0)
						{
							if (s_Render2Ddata->TextureSlotIndex >= Render2Ddata::MaxTextureSlots)
								Render2DFlushAndReset();

							textureIndex = s_Render2Ddata->TextureSlotIndex;
							s_Render2Ddata->TextureSlots[s_Render2Ddata->TextureSlotIndex] = sprite.Texture->GetTexture();
							s_Render2Ddata->TextureSlotIndex++;
						}
					}

					glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation);
					transform = glm::scale(transform, { tc.Scale.x, tc.Scale.y, 1.0f });

					for (int i = 0; i < quadVertexCount; i++)
					{
						QuadVertex vertex{};
						vertex.Position = transform * s_Render2Ddata->QuadVertexPositions[i];
						vertex.Color = sprite.Color;
						vertex.TexCoord = texcoord[i];
						vertex.TexIndex = textureIndex;

						s_Render2Ddata->QuadVertexBufferBase.push_back(vertex);
					}
					s_Render2Ddata->QuadIndexCount += 6;
				}
			}
		}
	}

	void Renderer::End()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(s_RenderData->CalledBegin, "Did you call Begin()?")
		s_RenderData->CalledBegin = false;

		if (s_RenderData->Settings.Renderer2D)
		{
			Render2DFlush();
		}

		FinalPass();
	}

	RenderSettings& Renderer::GetSettings()
	{
		YM_CORE_ASSERT(s_RenderData)

		return s_RenderData->Settings;
	}

	void Renderer::Shutdown()
	{
		YM_PROFILE_FUNCTION()

		delete s_Render2Ddata;
		delete s_RenderData;
	}

	void Renderer::Render2DInit()
	{
		YM_PROFILE_FUNCTION()

		// ----------------------------
		// QUAD stuff

		s_Render2Ddata->QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");

		s_Render2Ddata->QuadVertexArray = VertexArray::Create();

		s_Render2Ddata->QuadVertexBuffer = VertexBuffer::Create(nullptr, Render2Ddata::MaxVertices * sizeof(QuadVertex), BufferUsage::DYNAMIC);
		s_Render2Ddata->QuadVertexBuffer->SetLayout({
			{ DataType::Float3, "a_Position" },
			{ DataType::Float4, "a_Color"	 },
			{ DataType::Float2, "a_TexCoord" },
			{ DataType::Int , "a_TexIndex"   },
			});
		s_Render2Ddata->QuadVertexArray->AddVertexBuffer(s_Render2Ddata->QuadVertexBuffer);

		uint32_t* quadIndices = new uint32_t[Render2Ddata::MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < Render2Ddata::MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 0;
			quadIndices[i + 4] = offset + 2;
			quadIndices[i + 5] = offset + 3;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, Render2Ddata::MaxIndices);
		s_Render2Ddata->QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		s_Render2Ddata->QuadShader->AddVertexArray(s_Render2Ddata->QuadVertexArray);

		s_Render2Ddata->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_Render2Ddata->QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		s_Render2Ddata->QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		s_Render2Ddata->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_Render2Ddata->QuadDescriptorSet = DescriptorSet::Create(s_Render2Ddata->QuadShader);

		// ----------------------------
		// OTHER stuff

		s_Render2Ddata->TextureSlots.fill(s_RenderData->WhiteTexture);

		s_Render2Ddata->CameraUniformBuffer = UniformBuffer::Create(sizeof(Render2Ddata::CameraData));
	}

	void Renderer::Render2DFlush()
	{
		YM_PROFILE_FUNCTION()
			
		if (s_Render2Ddata->QuadIndexCount)
		{
			YM_PROFILE_SCOPE("QuadFlush")

			s_Render2Ddata->QuadPipeline->Begin();

			RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

			s_Render2Ddata->QuadDescriptorSet->Bind(0);
			s_Render2Ddata->QuadDescriptorSet->UploadUniform(0, s_Render2Ddata->CameraUniformBuffer);

			s_Render2Ddata->QuadDescriptorSet->Bind(1);
			s_Render2Ddata->QuadDescriptorSet->UploadTexture2D(0, s_Render2Ddata->TextureSlots.data(), (uint32_t)s_Render2Ddata->TextureSlots.size());

			s_Render2Ddata->QuadVertexBuffer->SetData(s_Render2Ddata->QuadVertexBufferBase.data(), s_Render2Ddata->QuadVertexBufferBase.size() * sizeof(QuadVertex));
			RendererCommand::DrawIndexed(s_Render2Ddata->QuadVertexArray, s_Render2Ddata->QuadIndexCount);

			s_Render2Ddata->QuadPipeline->End();
		}
	}

	void Renderer::Render2DStartBatch()
	{
		YM_PROFILE_FUNCTION()

		s_Render2Ddata->QuadIndexCount = 0;
		s_Render2Ddata->TextureSlotIndex = 1;

		s_Render2Ddata->QuadVertexBufferBase.clear();
	}

	void Renderer::Render2DFlushAndReset()
	{
		YM_PROFILE_FUNCTION()

		Render2DFlush();

		Render2DStartBatch();
	}

	void Renderer::FinalPass()
	{
		YM_PROFILE_FUNCTION()

		if (!s_RenderData->SwapchainTarget)
		{
			RendererCommand::ClearRenderTarget(s_RenderData->RenderTarget, s_RenderData->ClearColor);
		}

		PipelineCreateInfo pci{};
		pci.Shader = s_RenderData->FinalPassShader;
		pci.TransparencyEnabled = false;
		pci.DepthTest = false;
		pci.DepthWrite = false;
		pci.PolygonMode = PolygonMode::FILL;
		pci.ClearTargets = true;
		std::memcpy(pci.ClearColor, glm::value_ptr(s_RenderData->ClearColor), 4 * sizeof(float));
		pci.SwapchainTarget = true;
		if (!s_RenderData->SwapchainTarget)
		{
			pci.ColorTargets[0] = s_RenderData->RenderTarget;
			pci.SwapchainTarget = false;
			pci.ClearTargets = false;
		}
		s_RenderData->FinalPassPipeline = Pipeline::Get(pci);

		s_RenderData->FinalPassPipeline->Begin();

		RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

		s_RenderData->FinalPassDescriptorSet->Bind(0);
		s_RenderData->FinalPassDescriptorSet->UploadTexture2D(0, s_RenderData->MainTexture);

		RendererCommand::Draw(nullptr, 6);

		s_RenderData->FinalPassPipeline->End();
	}
}