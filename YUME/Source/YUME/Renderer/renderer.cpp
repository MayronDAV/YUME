#include "YUME/yumepch.h"
#include "YUME/Renderer/renderer.h"
#include "YUME/Scene/scene.h"
#include "YUME/Scene/entity.h"
#include "YUME/Scene/entity_manager.h"
#include "shader.h"
#include "descriptor_set.h"
#include "renderer_command.h"
#include "storage_buffer.h"
#include "YUME/Core/application.h"
#include "YUME/Utils/timer.h"
#include "buffer.h"
#include "YUME/Scene/Component/components_3D.h"


#include <glm/gtc/type_ptr.hpp>




namespace YUME
{

	struct RenderData
	{
		static const uint8_t MaxTextureSlots = 32;

		bool CalledBegin = false;
		glm::vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		Ref<Texture2D> MainTexture  = nullptr;
		Ref<Texture2D> DepthTexture = nullptr;
		Ref<Texture2D> WhiteTexture = nullptr;

		bool SwapchainTarget = true; 
		uint32_t Width;
		uint32_t Height;

		Ref<Shader> FinalPassShader				  = nullptr;
		Ref<Pipeline> FinalPassPipeline			  = nullptr;
		Ref<DescriptorSet> FinalPassDescriptorSet = nullptr;

		RenderSettings Settings = {};

		struct CameraData
		{
			glm::mat4 ViewProjection;
			glm::vec3 Position;
		};
		CameraData CameraBuffer{ glm::mat4(1.0f), glm::vec3(0.0f) };
		Ref<UniformBuffer> CameraUniformBuffer = nullptr;

		PolygonMode DrawPolygonMode = PolygonMode::FILL;

		Renderer::Statistics Stats;
	};

	struct QuadData
	{
		static const uint32_t MaxQuads		  = 5000;
		static const uint32_t MaxQuadVertices = MaxQuads * 4;
		static const uint32_t MaxQuadIndices  = MaxQuads * 6;

		Ref<IndexBuffer> IndexBuffer   = nullptr;
		Ref<VertexBuffer> VertexBuffer = nullptr;

		Ref<Shader> Shader = nullptr;
		std::array<Ref<DescriptorSet>, 2> DescriptorSets;

		Ref<Pipeline> Pipeline = nullptr;

		uint32_t IndexCount = 0;

		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
			glm::vec2 TexCoord;
			int TexIndex = 0;
		};
		std::vector<QuadVertex> VertexBufferBase;

		std::array<glm::vec4, 4> VertexPositions;

		std::array<Ref<Texture2D>, RenderData::MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		void Init(const std::string& p_ShaderPath = "assets/shaders/Renderer2D_Quad.glsl");
		void Flush();
		void Begin(bool p_CustomPCI = false, const PipelineCreateInfo& p_PCI = {});
		void StartBatch();
		void FlushAndReset();
	};

	struct CircleData
	{
		static const uint32_t MaxCircles		= 5000;
		static const uint32_t MaxCircleVertices = MaxCircles * 4;
		static const uint32_t MaxCircleIndices  = MaxCircles * 6;

		Ref<IndexBuffer> IndexBuffer   = nullptr;
		Ref<VertexBuffer> VertexBuffer = nullptr;

		Ref<Shader> Shader = nullptr;
		std::array<Ref<DescriptorSet>, 2> DescriptorSets;

		Ref<Pipeline> Pipeline = nullptr;

		uint32_t IndexCount = 0;

		struct CircleVertex
		{
			glm::vec3 WorldPosition;
			glm::vec3 LocalPosition;
			glm::vec4 Color;
			float Thickness;
			float Fade;
			int TexIndex = 0;
		};
		std::vector<CircleVertex> VertexBufferBase;

		std::array<glm::vec4, 4> VertexPositions;

		std::array<Ref<Texture2D>, RenderData::MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		void Init(const std::string& p_ShaderPath = "assets/shaders/Renderer2D_Circle.glsl");
		void Flush();
		void Begin(bool p_CustomPCI = false, const PipelineCreateInfo& p_PCI = {});
		void StartBatch();
		void FlushAndReset();
	};

	struct ModelData
	{
		Ref<Shader>		   Shader		  = nullptr;
		Ref<DescriptorSet> DescriptorSet  = nullptr;
		Ref<Pipeline>	   Pipeline	      = nullptr;


		void Init(const std::string& p_ShaderPath = "assets/shaders/solid_shader.glsl");
		void Begin(bool p_CustomPCI = false, const PipelineCreateInfo& p_PCI = {});
	};

	struct ForwardPBRData
	{
		static const uint8_t MAX_LIGHTS = 16;

		Ref<Shader>		   Shader		  = nullptr;
		Ref<DescriptorSet> DescriptorSet  = nullptr;
		Ref<Pipeline>	   Pipeline		  = nullptr;

		struct Light
		{
			glm::vec4		  Position{ 0.0f };
			glm::vec4		  Color{ 1.0f };
			glm::vec4		  Direction{ 0.0f };
			alignas(16) float Type = 0.0f;
		};
		struct LightBufferData
		{
			Light Lights[MAX_LIGHTS];
			int		   NumLights = 0;
		} LightBuffer;
		Ref<UniformBuffer> LightUBO = nullptr;

		void Init(const std::string& p_ShaderPath = "assets/shaders/pbr_shader.glsl");
		void Begin(bool p_CustomPCI = false, const PipelineCreateInfo& p_PCI = {});
	};

	struct OITData
	{
		Ref<Texture2D> OpaqueTexture = nullptr;
		Ref<Texture2D> AccumTexture  = nullptr;
		Ref<Texture2D> RevealTexture = nullptr;
		Ref<Texture2D> DepthTexture  = nullptr;

		Ref<Pipeline> CompositePipeline			  = nullptr;
		Ref<Shader> CompositeShader				  = nullptr;
		Ref<DescriptorSet> CompositeDescriptorSet = nullptr;

		CircleData* OpaqueCircles	   = nullptr;
		CircleData* TransparentCircles = nullptr;

		QuadData* OpaqueQuads	   = nullptr;
		QuadData* TransparentQuads = nullptr;

		void Init();
		void Begin();
		void CompositePass();

		~OITData()
		{
			if (OpaqueCircles)
				delete OpaqueCircles;
			if (OpaqueQuads)
				delete OpaqueQuads;

			if (TransparentCircles)
				delete TransparentCircles;
			if (TransparentQuads)
				delete TransparentQuads;
		}
	};


	static RenderData* s_RenderData;
	static QuadData* s_QuadData;
	static CircleData* s_CircleData;
	static ModelData* s_ModelData;
	static ForwardPBRData* s_ForwardPBR;
	static OITData* s_OITData;

	void Renderer::Shutdown()
	{
		YM_PROFILE_FUNCTION()

		if (s_QuadData)
			delete s_QuadData;
		if (s_CircleData)
			delete s_CircleData;
		if (s_ModelData)
			delete s_ModelData;
		if (s_ForwardPBR)
			delete s_ForwardPBR;
		if (s_OITData)
			delete s_OITData;

		Material::DestroyDefaultTextures();

		delete s_RenderData;
	}


	void Renderer::Init()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE("Renderer Initialized!")

		s_RenderData = new RenderData();

		s_RenderData->FinalPassShader = Shader::Create("assets/shaders/FinalPassShader.glsl");
		auto shader = Shader::Create("assets/shaders/pbr_shader.glsl");

		s_RenderData->FinalPassDescriptorSet = DescriptorSet::Create({/* Set */ 0, s_RenderData->FinalPassShader});

		uint8_t whiteData[] = { 255, 255, 255, 255 };
		s_RenderData->WhiteTexture = Texture2D::Create({}, whiteData, sizeof(whiteData));

		Material::CreateDefaultTextures();
		
		s_RenderData->CameraUniformBuffer = UniformBuffer::Create(sizeof(RenderData::CameraData));

		if (s_RenderData->Settings.OIT)
		{
			s_OITData = new OITData();

			s_OITData->Init();
		}

		if (s_RenderData->Settings.Renderer2D)
		{
			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.Renderer2D_Quad)
			{
				s_QuadData = new QuadData();

				s_QuadData->Init();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.Renderer2D_Circle)
			{
				s_CircleData = new CircleData();

				s_CircleData->Init();
			}
		}

		if (s_RenderData->Settings.Renderer3D)
		{
			if (!s_RenderData->Settings.OIT)
			{
				s_ModelData = new ModelData();

				s_ModelData->Init();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.PBR)
			{
				s_ForwardPBR = new ForwardPBRData();

				s_ForwardPBR->Init();
			}
		}
	}

	void Renderer::Begin(const RendererBeginInfo& p_BeginInfo)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(!s_RenderData->CalledBegin, "Did you call End()?")
		YM_CORE_VERIFY(p_BeginInfo.Width != 0 && p_BeginInfo.Height != 0)

		s_RenderData->CalledBegin = true;
		s_RenderData->ClearColor = p_BeginInfo.ClearColor;
		s_RenderData->SwapchainTarget = p_BeginInfo.SwapchainTarget;
		s_RenderData->Width = p_BeginInfo.Width;
		s_RenderData->Height = p_BeginInfo.Height;

		TextureSpecification texSpec{};
		texSpec.Width = s_RenderData->Width;
		texSpec.Height = s_RenderData->Height;
		texSpec.Format = TextureFormat::RGBA32_FLOAT;
		texSpec.Usage = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		texSpec.GenerateMips = false;
		texSpec.RenderTarget = true;
		texSpec.AnisotropyEnable = false;
		texSpec.DebugName = "MainTexture";
		s_RenderData->MainTexture = Texture2D::Get(texSpec);

		texSpec.RenderTarget = false;
		texSpec.Format = TextureFormat::D32_FLOAT;
		texSpec.Usage = TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
		texSpec.DebugName = "MainDepthTexture";
		s_RenderData->DepthTexture = Texture2D::Get(texSpec);

		RendererCommand::ClearRenderTarget(s_RenderData->MainTexture, s_RenderData->ClearColor);
		RendererCommand::ClearRenderTarget(s_RenderData->DepthTexture);

		if (s_RenderData->Settings.OIT)
		{
			s_OITData->Begin();
		}

		s_RenderData->CameraBuffer.ViewProjection = p_BeginInfo.MainCamera.GetViewProjection();
		s_RenderData->CameraBuffer.Position = p_BeginInfo.MainCamera.GetPosition();
		s_RenderData->CameraUniformBuffer->SetData(&s_RenderData->CameraBuffer, sizeof(RenderData::CameraData));

		if (s_RenderData->Settings.Renderer2D)
		{
			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.Renderer2D_Quad)
			{
				s_QuadData->Begin();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.Renderer2D_Circle)
			{
				s_CircleData->Begin();
			}
		}

		if (s_RenderData->Settings.Renderer3D)
		{
			if (!s_RenderData->Settings.OIT)
			{
				s_ModelData->Begin();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.PBR)
			{
				s_ForwardPBR->Begin();
			}
		}

		ResetStats();
	}

	void Renderer::End()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(s_RenderData->CalledBegin, "Did you call Begin()?")
		s_RenderData->CalledBegin = false;

		Timer endTime;
		endTime.Start();

		if (s_RenderData->Settings.Renderer2D)
		{
			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.Renderer2D_Circle)
			{
				s_CircleData->Flush();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.Renderer2D_Quad)
			{
				s_QuadData->Flush();
			}
		}

		if (s_RenderData->Settings.OIT)
		{
			s_OITData->OpaqueCircles->Flush();
			s_OITData->OpaqueQuads->Flush();

			s_OITData->TransparentCircles->Flush();
			s_OITData->TransparentQuads->Flush();

			s_OITData->CompositePass();
		}

		if (s_RenderData->SwapchainTarget)
		{
			FinalPass();
		}

		endTime.Stop();
		s_RenderData->Stats.EndTimeMs += endTime.Elapsed() * 1000.0;
	}

	void Renderer::RenderScene(Scene* p_Scene)
	{
		YM_PROFILE_FUNCTION()

		auto enttManager = p_Scene->GetEntityManager();
		auto& registry = p_Scene->GetRegistry();

		Timer renderSceneTime;
		renderSceneTime.Start();

		if (s_RenderData->Settings.Renderer3D)
		{
			YM_PROFILE_SCOPE("Renderer3D")

			bool pbr = s_RenderData->Settings.PBR;
			Ref<Pipeline> pipeline = nullptr;
			Ref<DescriptorSet> descriptorSet = nullptr;
			Ref<Shader> shader = nullptr;
			if (!s_RenderData->Settings.OIT && pbr)
			{
				pipeline = s_ForwardPBR->Pipeline;
				descriptorSet = s_ForwardPBR->DescriptorSet;
				shader = s_ForwardPBR->Shader;
			}
			else if (!s_RenderData->Settings.OIT)
			{
				pipeline = s_ModelData->Pipeline;
				descriptorSet = s_ModelData->DescriptorSet;
				shader = s_ModelData->Shader;
			}

			descriptorSet->SetUniformData("u_Camera", s_RenderData->CameraUniformBuffer);
			descriptorSet->Upload();

			if (pbr)
			{
				auto group = enttManager->GetEntitiesWithTypes<LightComponent>();
				int index = 0;
				for (auto entt : group)
				{
					if (index >= ForwardPBRData::MAX_LIGHTS)
						break;

					Entity entity = { entt, p_Scene };
					auto& lc = entity.GetComponent<LightComponent>();
					auto& tc = entity.GetComponent<TransformComponent>();

					ForwardPBRData::Light light{};
					light.Color = glm::vec4(lc.Color, 1.0f);
					light.Position = glm::vec4(tc.Translation, 1.0f);
					light.Direction = glm::vec4(lc.Direction, 1.0f);
					light.Type = float(lc.Type);

					s_ForwardPBR->LightBuffer.Lights[index] = light;
					index++;
				}
				s_ForwardPBR->LightBuffer.NumLights = index;
				s_ForwardPBR->LightUBO->SetData(&s_ForwardPBR->LightBuffer, sizeof(ForwardPBRData::LightBufferData));

				descriptorSet->SetUniformData("LightBuffer", s_ForwardPBR->LightUBO);
				descriptorSet->Upload();
			}


			auto group = enttManager->GetEntitiesWithTypes<ModelComponent>();
			for (auto entt : group)
			{
				Entity entity = { entt, p_Scene };
				auto& mc = entity.GetComponent<ModelComponent>();
				auto& tc = entity.GetComponent<TransformComponent>();

				pipeline->Begin();

				RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

				RendererCommand::BindDescriptorSets(&descriptorSet);

				for (const auto& mesh : mc.ModelRef->GetMeshes())
				{
					auto transform = tc.GetTransform();
					shader->SetPushValue("Transform", &transform);

					mesh->BindMaterial(shader, pbr);

					shader->BindPushConstants();
					RendererCommand::DrawMesh(mesh);
					s_RenderData->Stats.DrawCalls++;
				}

				pipeline->End();
			}
		}

		if (s_RenderData->Settings.Renderer2D)
		{
			YM_PROFILE_SCOPE("Renderer2D")

			auto group = enttManager->GetEntitiesWithTypes<SpriteComponent>();
			for (auto entt : group)
			{
				Entity entity = { entt, p_Scene };
				auto& sprite = entity.GetComponent<SpriteComponent>();
				auto& shape = entity.GetComponent<ShapeComponent>();
				auto& tc = entity.GetComponent<TransformComponent>();

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

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation);
				transform = glm::scale(transform, { tc.Scale.x, tc.Scale.y, 1.0f });

				float opacityThreshold = 0.98f;

				bool isTransparent = sprite.Type == SurfaceType::Transparent || sprite.Color.a < opacityThreshold;

				if (s_RenderData->Settings.Renderer2D_Quad && shape.IsShape<Square>())
				{
					QuadData* quadData = nullptr;
					if (s_RenderData->Settings.OIT)
					{
						if (isTransparent)
						{
							quadData = s_OITData->TransparentQuads;
						}
						else
						{
							quadData = s_OITData->OpaqueQuads;
						}
					}
					else
					{
						quadData = s_QuadData;
					}

					if (quadData->IndexCount >= quadData->MaxQuadIndices)
					{
						quadData->FlushAndReset();
					}


					int textureIndex = 0; // White texture
					if (sprite.Texture != nullptr)
					{
						for (uint32_t i = 1; i < quadData->TextureSlotIndex; i++)
						{
							if (*quadData->TextureSlots[i].get() == *sprite.Texture->GetTexture().get())
							{
								textureIndex = i;
								break;
							}
						}

						if (textureIndex == 0)
						{
							if (quadData->TextureSlotIndex >= RenderData::MaxTextureSlots)
								quadData->FlushAndReset();

							textureIndex = quadData->TextureSlotIndex;
							quadData->TextureSlots[textureIndex] = sprite.Texture->GetTexture();
							quadData->TextureSlotIndex++;
						}
					}

					for (int i = 0; i < quadVertexCount; i++)
					{
						QuadData::QuadVertex vertex{};
						vertex.Position = transform * quadData->VertexPositions[i];
						vertex.Color = sprite.Color;
						vertex.TexCoord = texcoord[i];
						vertex.TexIndex = textureIndex;

						quadData->VertexBufferBase.push_back(vertex);
					}
					quadData->IndexCount += 6;
					s_RenderData->Stats.QuadCount++;
				}
				else if (shape.IsShape<Circle>() && s_RenderData->Settings.Renderer2D_Circle)
				{
					auto& circle = shape.Get<Circle>();
					CircleData* circleData = nullptr;
					if (s_RenderData->Settings.OIT)
					{
						if (isTransparent)
						{
							circleData = s_OITData->TransparentCircles;
						}
						else
						{
							circleData = s_OITData->OpaqueCircles;
						}
					}
					else
					{
						circleData = s_CircleData;
					}

					if (circleData->IndexCount >= circleData->MaxCircleIndices)
					{
						circleData->FlushAndReset();
					}

					int textureIndex = 0; // White texture
					if (sprite.Texture != nullptr)
					{
						for (uint32_t i = 1; i < circleData->TextureSlotIndex; i++)
						{
							if (*circleData->TextureSlots[i].get() == *sprite.Texture->GetTexture().get())
							{
								textureIndex = i;
								break;
							}
						}

						if (textureIndex == 0)
						{
							if (circleData->TextureSlotIndex >= RenderData::MaxTextureSlots)
								circleData->FlushAndReset();

							textureIndex = circleData->TextureSlotIndex;
							circleData->TextureSlots[textureIndex] = sprite.Texture->GetTexture();
							circleData->TextureSlotIndex++;
						}
					}

					for (size_t i = 0; i < quadVertexCount; i++)
					{
						CircleData::CircleVertex vertex{};
						vertex.WorldPosition = transform * circleData->VertexPositions[i];
						vertex.LocalPosition = circleData->VertexPositions[i] * 2.0f;
						vertex.Color = sprite.Color;
						vertex.Thickness = circle.Thickness;
						vertex.Fade = circle.Fade;
						vertex.TexIndex = textureIndex;

						circleData->VertexBufferBase.push_back(vertex);
					}
					circleData->IndexCount += 6;
					s_RenderData->Stats.CircleCount++;
				}
			}
		}

		renderSceneTime.Stop();
		s_RenderData->Stats.RenderSceneTimeMs += renderSceneTime.Elapsed() * 1000.0;
	}


	RenderSettings& Renderer::GetSettings()
	{
		YM_CORE_ASSERT(s_RenderData)

		return s_RenderData->Settings;
	}

	Ref<Texture2D> Renderer::GetRenderTexture()
	{
		return s_RenderData->MainTexture;
	}


	void Renderer::SetPolygonMode(PolygonMode p_Mode)
	{
		s_RenderData->DrawPolygonMode = p_Mode;
	}



	Renderer::Statistics Renderer::GetStats()
	{
		return s_RenderData->Stats;
	}

	void Renderer::ResetStats()
	{
		s_RenderData->Stats = Statistics();
	}

	void Renderer::FinalPass()
	{
		YM_PROFILE_FUNCTION()

		PipelineCreateInfo pci{};
		pci.Shader = s_RenderData->FinalPassShader;
		pci.TransparencyEnabled = false;
		pci.DepthTest = false;
		pci.DepthWrite = false;
		pci.PolygonMode = PolygonMode::FILL;

		if (s_RenderData->SwapchainTarget)
		{
			pci.SwapchainTarget = true;
			pci.ClearTargets = true;
			std::memcpy(pci.ClearColor, glm::value_ptr(s_RenderData->ClearColor), 4 * sizeof(float));
		}
		pci.DebugName = "FinalPassPipeline";

		s_RenderData->FinalPassPipeline = Pipeline::Get(pci);

		s_RenderData->FinalPassPipeline->Begin();

		RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

		s_RenderData->FinalPassDescriptorSet->SetTexture2D("u_Texture", s_RenderData->MainTexture);
		s_RenderData->FinalPassDescriptorSet->Upload();

		RendererCommand::BindDescriptorSets(&s_RenderData->FinalPassDescriptorSet);


		RendererCommand::Draw(nullptr, 6);
		s_RenderData->Stats.DrawCalls++;

		s_RenderData->FinalPassPipeline->End();
	}

	void QuadData::Init(const std::string& p_ShaderPath)
	{
		YM_PROFILE_FUNCTION()

		Shader = Shader::Create(p_ShaderPath);
		Shader->SetLayout({
			{ DataType::Float3, "a_Position" },
			{ DataType::Float4, "a_Color"	 },
			{ DataType::Float2, "a_TexCoord" },
			{ DataType::Int ,   "a_TexIndex" },
		});

		VertexBuffer = VertexBuffer::Create(nullptr, MaxQuadVertices * sizeof(QuadVertex), BufferUsage::DYNAMIC);

		uint32_t* quadIndices = new uint32_t[MaxQuadIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < MaxQuadIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 0;
			quadIndices[i + 4] = offset + 2;
			quadIndices[i + 5] = offset + 3;

			offset += 4;
		}

		IndexBuffer = IndexBuffer::Create(quadIndices, MaxQuadIndices);
		delete[] quadIndices;

		VertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		VertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
		VertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
		VertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		for (size_t i = 0; i < 2; i++)
		{
			DescriptorSpec spec;
			spec.Set = (uint32_t)i;
			spec.Shader = Shader;
			DescriptorSets[i] = DescriptorSet::Create(spec);
		}

		TextureSlots.fill(s_RenderData->WhiteTexture);
	}

	void QuadData::Flush()
	{
		YM_PROFILE_FUNCTION()

		if (IndexCount)
		{	
			Pipeline->Begin();

			RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

			DescriptorSets[0]->SetUniformData("u_Camera", s_RenderData->CameraUniformBuffer);
			DescriptorSets[0]->Upload();
			DescriptorSets[1]->SetTexture2D("u_Textures", TextureSlots.data(), (uint32_t)TextureSlots.size());
			DescriptorSets[1]->Upload();


			VertexBuffer->SetData(VertexBufferBase.data(), VertexBufferBase.size() * sizeof(QuadVertex));

			RendererCommand::BindDescriptorSets(DescriptorSets.data(), (uint32_t)DescriptorSets.size());
			RendererCommand::DrawIndexed(VertexBuffer, IndexBuffer);
			s_RenderData->Stats.DrawCalls++;

			Pipeline->End();
		}
	}

	void QuadData::Begin(bool p_CustomPCI, const PipelineCreateInfo& p_PCI)
	{
		PipelineCreateInfo pci{};
		if (p_CustomPCI)
		{
			pci = p_PCI;
		}
		else
		{
			pci.ColorTargets[0] = s_RenderData->MainTexture;
			pci.BlendModes[0] = BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			pci.PolygonMode = PolygonMode::FILL;
			pci.ClearTargets = false;
			pci.SwapchainTarget = false;
			pci.DepthTarget = s_RenderData->DepthTexture;
			pci.DepthTest = true;
			pci.DepthWrite = true;
			pci.DebugName = "QuadPipeline";
		}
		pci.Shader = Shader;

		Pipeline = Pipeline::Get(pci);

		StartBatch();
	}

	void QuadData::StartBatch()
	{
		YM_PROFILE_FUNCTION()

		IndexCount = 0;
		TextureSlotIndex = 1;
		VertexBufferBase.clear();
	}

	void QuadData::FlushAndReset()
	{
		YM_PROFILE_FUNCTION()

		Flush();

		StartBatch();
	}

	void CircleData::Init(const std::string& p_ShaderPath)
	{
		YM_PROFILE_FUNCTION()

		Shader = Shader::Create(p_ShaderPath);
		Shader->SetLayout({
			{ DataType::Float3, "a_WorldPosition" },
			{ DataType::Float3, "a_LocalPosition" },
			{ DataType::Float4, "a_Color"		  },
			{ DataType::Float,  "a_Thickness"	  },
			{ DataType::Float,  "a_Fade"		  },
			{ DataType::Int,    "a_TexIndex"	  }
		});

		VertexBuffer = VertexBuffer::Create(nullptr, MaxCircleVertices * sizeof(CircleVertex), BufferUsage::DYNAMIC);

		uint32_t* circleIndices = new uint32_t[MaxCircleIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < MaxCircleIndices; i += 6)
		{
			circleIndices[i + 0] = offset + 0;
			circleIndices[i + 1] = offset + 1;
			circleIndices[i + 2] = offset + 2;

			circleIndices[i + 3] = offset + 0;
			circleIndices[i + 4] = offset + 2;
			circleIndices[i + 5] = offset + 3;

			offset += 4;
		}

		IndexBuffer = IndexBuffer::Create(circleIndices, MaxCircleIndices);
		delete[] circleIndices;

		VertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		VertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		VertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		VertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };


		for (size_t i = 0; i < 2; i++)
		{
			DescriptorSpec spec;
			spec.Set = (uint32_t)i;
			spec.Shader = Shader;
			DescriptorSets[i] = DescriptorSet::Create(spec);
		}

		TextureSlots.fill(s_RenderData->WhiteTexture);
	}

	void CircleData::Flush()
	{
		YM_PROFILE_FUNCTION()

		if (IndexCount)
		{
			Pipeline->Begin();

			RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

			DescriptorSets[0]->SetUniformData("u_Camera", s_RenderData->CameraUniformBuffer);
			DescriptorSets[0]->Upload();

			DescriptorSets[1]->SetTexture2D("u_Textures", TextureSlots.data(), (uint32_t)TextureSlots.size());
			DescriptorSets[1]->Upload();

			VertexBuffer->SetData(VertexBufferBase.data(), VertexBufferBase.size() * sizeof(CircleVertex));

			RendererCommand::BindDescriptorSets(DescriptorSets.data(), (uint32_t)DescriptorSets.size());
			RendererCommand::DrawIndexed(VertexBuffer, IndexBuffer);
			s_RenderData->Stats.DrawCalls++;

			Pipeline->End();
		}
	}

	void CircleData::Begin(bool p_CustomPCI, const PipelineCreateInfo& p_PCI)
	{
		PipelineCreateInfo pci{};
		if (p_CustomPCI)
		{
			pci = p_PCI;
		}
		else
		{
			pci.ColorTargets[0] = s_RenderData->MainTexture;
			pci.BlendModes[0] = BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			pci.PolygonMode = PolygonMode::FILL;
			pci.ClearTargets = false;
			pci.SwapchainTarget = false;
			pci.DepthTarget = s_RenderData->DepthTexture;
			pci.DepthTest = true;
			pci.DepthWrite = true;
			pci.DebugName = "CirclePipeline";
		}

		pci.Shader = Shader;


		Pipeline = Pipeline::Get(pci);

		StartBatch();
	}

	void CircleData::StartBatch()
	{
		YM_PROFILE_FUNCTION()

		IndexCount = 0;
		TextureSlotIndex = 1;
		VertexBufferBase.clear();
	}

	void CircleData::FlushAndReset()
	{
		YM_PROFILE_FUNCTION()

		Flush();

		StartBatch();
	}

	void OITData::Init()
	{
		YM_PROFILE_FUNCTION()

		CompositeShader = Shader::Create("assets/shaders/oit/composite_shader.glsl");
		CompositeDescriptorSet = DescriptorSet::Create({ /* Set */ 0, CompositeShader});

		if (s_RenderData->Settings.Renderer2D_Circle)
		{
			OpaqueCircles = new CircleData();
			TransparentCircles = new CircleData();

			OpaqueCircles->Init();
			TransparentCircles->Init("assets/shaders/oit/transparent_circle_shader.glsl");
		}

		if (s_RenderData->Settings.Renderer2D_Quad)
		{
			OpaqueQuads = new QuadData();
			TransparentQuads = new QuadData();

			OpaqueQuads->Init();
			TransparentQuads->Init("assets/shaders/oit/transparent_quad_shader.glsl");
		}
	}

	void OITData::Begin()
	{
		YM_PROFILE_FUNCTION()

		TextureSpecification texSpec{};
		texSpec.Width = s_RenderData->Width;
		texSpec.Height = s_RenderData->Height;
		texSpec.Format = TextureFormat::RGBA16_FLOAT;
		texSpec.Usage = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		texSpec.GenerateMips = false;
		texSpec.RenderTarget = true;
		texSpec.DebugName = "OpaqueTexture";
		OpaqueTexture = Texture2D::Get(texSpec);

		texSpec.AnisotropyEnable = false;
		texSpec.Format = TextureFormat::RGBA32_FLOAT;
		texSpec.DebugName = "AccumTexture";
		AccumTexture = Texture2D::Get(texSpec);

		texSpec.Format = TextureFormat::R32_FLOAT;
		texSpec.DebugName = "RevealTexture";
		RevealTexture = Texture2D::Get(texSpec);

		texSpec.Format = TextureFormat::D32_FLOAT;
		texSpec.Usage = TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
		texSpec.RenderTarget = false;
		texSpec.DebugName = "DepthTexture";
		DepthTexture = Texture2D::Get(texSpec);

		RendererCommand::ClearRenderTarget(OpaqueTexture, s_RenderData->ClearColor);
		RendererCommand::ClearRenderTarget(AccumTexture,  { 0.0f, 0.0f, 0.0f, 0.0f });
		RendererCommand::ClearRenderTarget(RevealTexture, { 1.0f, 0.0f, 0.0f, 0.0f });
		RendererCommand::ClearRenderTarget(DepthTexture);

		// Composite
		{
			PipelineCreateInfo pci{};
			pci.Shader = CompositeShader;
			pci.ColorTargets[0] = s_RenderData->MainTexture;
			pci.TransparencyEnabled = false;
			pci.DepthTest = false;
			pci.DepthWrite = false;
			pci.ClearTargets = false;
			pci.SwapchainTarget = false;
			pci.PolygonMode = PolygonMode::FILL;
			pci.DebugName = "CompositePipeline";

			CompositePipeline = Pipeline::Get(pci);
		}


		PipelineCreateInfo pci{};
		pci.ClearTargets = false;
		pci.SwapchainTarget = false;
		pci.PolygonMode = PolygonMode::FILL;
		pci.DepthTarget = DepthTexture;
		pci.DepthTest = true;

		// Opaque
		{
			pci.ColorTargets[0] = OpaqueTexture;
			pci.BlendModes[0] = BlendMode::None;
			pci.TransparencyEnabled = false;
			pci.DepthWrite = true;

			if (s_RenderData->Settings.Renderer2D_Circle)
			{
				pci.DebugName = "OpaqueCirclePipeline";
				OpaqueCircles->Begin(true, pci);
			}

			if (s_RenderData->Settings.Renderer2D_Quad)
			{
				pci.DebugName = "OpaqueQuadPipeline";
				OpaqueQuads->Begin(true, pci);
			}
		}

		// Transparent
		{
			pci.TransparencyEnabled = true;
			pci.ColorTargets[0] = AccumTexture;
			pci.ColorTargets[1] = RevealTexture;
			pci.BlendModes[0] = BlendMode::OneOne;
			pci.BlendModes[1] = BlendMode::ZeroOneMinusSrcColor;
			pci.DepthWrite = false;

			if (s_RenderData->Settings.Renderer2D_Circle)
			{
				pci.DebugName = "TransparentCirclePipeline";
				TransparentCircles->Begin(true, pci);
			}

			if (s_RenderData->Settings.Renderer2D_Quad)
			{
				pci.DebugName = "TransparentQuadPipeline";
				TransparentQuads->Begin(true, pci);
			}
		}

	}

	void OITData::CompositePass()
	{
		YM_PROFILE_FUNCTION()

		CompositePipeline->Begin();

		RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

		CompositeDescriptorSet->SetTexture2D("u_AccumTexture", AccumTexture);
		CompositeDescriptorSet->SetTexture2D("u_RevealTexture", RevealTexture);
		CompositeDescriptorSet->SetTexture2D("u_OpaqueTexture", OpaqueTexture);
		CompositeDescriptorSet->Upload();

		RendererCommand::BindDescriptorSets(&CompositeDescriptorSet);
		RendererCommand::Draw(nullptr, 6);
		s_RenderData->Stats.DrawCalls++;

		CompositePipeline->End();
	}

	void ModelData::Init(const std::string& p_ShaderPath)
	{
		Shader = Shader::Create(p_ShaderPath);
		Shader->SetLayout({
			{ DataType::Float3, "a_Position" },
			{ DataType::Float3, "a_Normal"   },
			{ DataType::Float2, "a_TexCoord" },
			{ DataType::Float4, "a_Color"	 },
		});

		DescriptorSet = DescriptorSet::Create({ /* Set */ 0, Shader });
	}

	void ModelData::Begin(bool p_CustomPCI, const PipelineCreateInfo& p_PCI)
	{
		PipelineCreateInfo pci{};
		if (p_CustomPCI)
		{
			pci = p_PCI;
		}
		else
		{
			pci.ColorTargets[0] = s_RenderData->MainTexture;
			pci.BlendModes[0] = BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			pci.PolygonMode = PolygonMode::FILL;
			pci.ClearTargets = false;
			pci.SwapchainTarget = false;
			pci.DepthTarget = s_RenderData->DepthTexture;
			pci.DepthTest = true;
			pci.DepthWrite = true;
			pci.DebugName = "ModelPipeline";
		}

		pci.Shader = Shader;


		Pipeline = Pipeline::Get(pci);
	}

	void ForwardPBRData::Init(const std::string& p_ShaderPath)
	{
		Shader = Shader::Create(p_ShaderPath);
		Shader->SetLayout({
			{ DataType::Float3, "a_Position" },
			{ DataType::Float3, "a_Normal"   },
			{ DataType::Float2, "a_TexCoord" },
			{ DataType::Float4, "a_Color"	 },
			});

		DescriptorSet = DescriptorSet::Create({ /* Set */ 0, Shader });

		LightUBO = UniformBuffer::Create(sizeof(LightBufferData));
	}

	void ForwardPBRData::Begin(bool p_CustomPCI, const PipelineCreateInfo& p_PCI)
	{
		PipelineCreateInfo pci{};
		if (p_CustomPCI)
		{
			pci = p_PCI;
		}
		else
		{
			pci.ColorTargets[0] = s_RenderData->MainTexture;
			pci.BlendModes[0] = BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			pci.PolygonMode = PolygonMode::FILL;
			pci.ClearTargets = false;
			pci.SwapchainTarget = false;
			pci.DepthTarget = s_RenderData->DepthTexture;
			pci.DepthTest = true;
			pci.DepthWrite = true;
			pci.DebugName = "ModelPBRPipeline";
		}

		pci.Shader = Shader;

		Pipeline = Pipeline::Get(pci);

		LightBuffer.NumLights = 0;
		for (int i = 0; i < MAX_LIGHTS; i++)
		{
			LightBuffer.Lights[i].Position = glm::vec4(0.0f);
			LightBuffer.Lights[i].Color = glm::vec4(1.0f);
			LightBuffer.Lights[i].Direction = glm::vec4(0.0f);
			LightBuffer.Lights[i].Type = 0.0f;
		}
	}
}