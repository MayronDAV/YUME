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
#include "YUME/Core/command_buffer.h"
#include "YUME/Scene/Component/components_3D.h"
#include "YUME/Utils/clock.h"
//#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"


#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>




namespace YUME
{

	struct RenderData
	{
		static const uint8_t MaxTextureSlots = 32;

		bool CalledBegin = false;
		glm::vec4 ClearColor{ 0.0f, 0.0f, 0.0f, 1.0f };
		Ref<Texture2D> MainTexture	= nullptr;
		Ref<Texture2D> DepthTexture	= nullptr;
		Ref<Texture2D> WhiteTexture	= nullptr;

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
			glm::vec4	   Position{ 0.0f };
			glm::vec4	   Color{ 1.0f };
			glm::vec4	   Direction{ 0.0f };
			glm::vec3	   AttenuationProps{ 1.0f, 0.09f, 0.032f };
			float		   Type = 0.0f;
		};
		struct LightBufferData
		{
			Light		   Lights[MAX_LIGHTS];
			int			   NumLights = 0;
		} LightBuffer;
		Ref<UniformBuffer> LightUBO = nullptr;

		void Init(const std::string& p_ShaderPath = "assets/shaders/pbr_shader.glsl");
		void Begin(bool p_CustomPCI = false, const PipelineCreateInfo& p_PCI = {});
	};
	
	struct ShadowData
	{
		uint32_t		   Resolution		  = 1024;
		bool			   EnableDebugView	  = false;

		Ref<Texture2D>	   DebugTexture		  = nullptr;
		Ref<Texture2D>	   ShadowMap		  = nullptr;

		Ref<Shader>		   DebugShader		  = nullptr;
		Ref<Pipeline>	   DebugPipeline	  = nullptr;
		Ref<DescriptorSet> DebugDescriptorSet = nullptr;

		Ref<Shader>		   Shader			  = nullptr;
		Ref<Pipeline>	   Pipeline			  = nullptr;
		Ref<DescriptorSet> DescriptorSet	  = nullptr;

		glm::vec3		   LightPosition	  = { -41.0f,  16.0f, -47.0f };
		glm::vec3		   LightDirection	  = {  45.0f, -45.0f, -90.0f };
		float			   LightFOV			  = 120.0f;

		struct DebugUBOData
		{
			int			   Mode				  = 0; // 0: perspective, 1: ortho
			float		   zNear			  = 1.0f;
			float		   zFar				  = 150.0f;
		} DebugUBOBuffer;
		Ref<UniformBuffer> DebugUBO			  = nullptr;

		struct UBOData
		{
			glm::mat4 LightSpaceMatrix{ 1.0f };
		} LightSpaceBuffer;
		Ref<UniformBuffer> LightSpaceUBO = nullptr;

		void Init();
		void Begin();
	};

	struct SkyboxData
	{
		Ref<TextureArray>  Skybox				   = nullptr;

		Ref<Shader>		   Shader				   = nullptr;
		Ref<Pipeline>	   Pipeline				   = nullptr;

		Ref<DescriptorSet> DescriptorSet		   = nullptr;

		struct BufferData
		{
			glm::mat4	   Projection			   = glm::mat4(1.0f);
			glm::mat4	   View					   = glm::mat4(1.0f);
		} Buffer;
		Ref<UniformBuffer> SkyboxUBO			   = nullptr;


		void Init();
		void Begin(const glm::mat4& p_Projection, const glm::mat4& p_View);
	};

	struct OITData
	{
		Ref<Texture2D>	   OpaqueTexture		   = nullptr;
		Ref<Texture2D>	   AccumTexture			   = nullptr;
		Ref<Texture2D>	   RevealTexture		   = nullptr;
		Ref<Texture2D>	   DepthTexture			   = nullptr;

		Ref<Pipeline>	   CompositePipeline	   = nullptr;
		Ref<Shader>		   CompositeShader		   = nullptr;
		Ref<DescriptorSet> CompositeDescriptorSet  = nullptr;

		CircleData*		   OpaqueCircles		   = nullptr;
		CircleData*		   TransparentCircles	   = nullptr;

		QuadData*		   OpaqueQuads			   = nullptr;
		QuadData*		   TransparentQuads		   = nullptr;

		//	TODO: Renderer3D oit

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
	static ShadowData* s_ShadowData;
	static SkyboxData* s_SkyboxData;
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
		if (s_ShadowData)
			delete s_ShadowData;
		if (s_SkyboxData)
			delete s_SkyboxData;
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
			if (s_RenderData->Settings.Skybox)
			{
				s_SkyboxData = new SkyboxData();

				s_SkyboxData->Init();
			}

			if (!s_RenderData->Settings.OIT && !s_RenderData->Settings.PBR)
			{
				s_ModelData = new ModelData();

				s_ModelData->Init();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.PBR)
			{
				s_ForwardPBR = new ForwardPBRData();

				s_ForwardPBR->Init();
			}

			if (s_RenderData->Settings.PBR)
			{
				s_ShadowData = new ShadowData();

				s_ShadowData->Init();
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
		texSpec.AnisotropyEnable = false;
		texSpec.DebugName = "MainTexture";
		s_RenderData->MainTexture = Texture2D::Get(texSpec);

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

		s_RenderData->CameraBuffer.ViewProjection = p_BeginInfo.MainCamera.GetProjection() * p_BeginInfo.MainCamera.GetView();
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
			if (s_RenderData->Settings.Skybox)
			{
				s_SkyboxData->Begin(p_BeginInfo.MainCamera.GetProjection(), p_BeginInfo.MainCamera.GetView());
			}

			if (!s_RenderData->Settings.OIT && !s_RenderData->Settings.PBR)
			{
				s_ModelData->Begin();
			}

			if (!s_RenderData->Settings.OIT && s_RenderData->Settings.PBR)
			{
				s_ForwardPBR->Begin();
			}

			if (s_RenderData->Settings.PBR)
			{
				s_ShadowData->Begin();
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
		auto commandBuffer = Application::Get().GetWindow().GetContext()->GetCurrentCommandBuffer();

		Timer renderSceneTime;
		renderSceneTime.Start();

		if (s_RenderData->Settings.Renderer3D)
		{
			YM_PROFILE_SCOPE("Renderer3D")

			if (s_RenderData->Settings.Skybox)
			{
				s_SkyboxData->Pipeline->Begin(commandBuffer);

				RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

				Ref<TextureArray> skybox = nullptr;
				registry.view<SkyboxComponent>().each(
				[&](auto p_Entt, const SkyboxComponent& p_SC)
				{
					if (p_SC.Skybox)
					{
						skybox = p_SC.Skybox;
						return;
					}
				});

				if (!skybox)
				{
					skybox = s_SkyboxData->Skybox;
				}

				s_SkyboxData->DescriptorSet->SetUniformData("u_Buffer", s_SkyboxData->SkyboxUBO);
				s_SkyboxData->DescriptorSet->Upload(commandBuffer);

				s_SkyboxData->DescriptorSet->SetTexture("u_Skybox", skybox);
				s_SkyboxData->DescriptorSet->Upload(commandBuffer);

				RendererCommand::BindDescriptorSets(commandBuffer, &s_SkyboxData->DescriptorSet);
				RendererCommand::Draw(commandBuffer, nullptr, 36);
				s_RenderData->Stats.DrawCalls++;

				s_SkyboxData->Pipeline->End(commandBuffer);
			}


			bool pbr = s_RenderData->Settings.PBR;

			Ref<Pipeline> pipeline = nullptr;
			Ref<DescriptorSet> descriptorSet = nullptr;
			Ref<Shader> shader = nullptr;
			if (!s_RenderData->Settings.OIT && pbr)
			{
				pipeline	  = s_ForwardPBR->Pipeline;
				descriptorSet = s_ForwardPBR->DescriptorSet;
				shader		  = s_ForwardPBR->Shader;
			}
			else if (!s_RenderData->Settings.OIT)
			{
				pipeline	  = s_ModelData->Pipeline;
				descriptorSet = s_ModelData->DescriptorSet;
				shader		  = s_ModelData->Shader;
			}

			descriptorSet->SetUniformData("u_Camera", s_RenderData->CameraUniformBuffer);
			descriptorSet->Upload(commandBuffer);

			if (pbr)
			{
				int index = 0;
				registry.view<TransformComponent, LightComponent>().each(
				[&](auto p_Entt, const TransformComponent& p_Transform, const LightComponent& p_Light)
				{
					if (index >= ForwardPBRData::MAX_LIGHTS)
						return;

					ForwardPBRData::Light light{};
					light.Color = glm::vec4(p_Light.Color, std::max(p_Light.Intensity, 0.0f));
					light.Position = glm::vec4(p_Transform.Transform.GetLocalTranslation(), 1.0f);
					light.Direction = glm::vec4(p_Light.Direction, 1.0f);
					light.AttenuationProps = glm::vec3(1.0f, p_Light.Linear, p_Light.Quadratic);
					light.Type = float(p_Light.Type);

					s_ForwardPBR->LightBuffer.Lights[index] = light;
					index++;
				});

				s_ForwardPBR->LightBuffer.NumLights = index;
				s_ForwardPBR->LightUBO->SetData(&s_ForwardPBR->LightBuffer, sizeof(ForwardPBRData::LightBufferData));

				descriptorSet->SetUniformData("LightBuffer", s_ForwardPBR->LightUBO);
				descriptorSet->Upload(commandBuffer);

				// Shadow
				{
					s_ShadowData->Pipeline->Begin(commandBuffer);

					RendererCommand::SetViewport(0, 0, s_ShadowData->Resolution, s_ShadowData->Resolution);

					glm::mat4 projection{ 1.0f };
					if (s_ShadowData->DebugUBOBuffer.Mode == 0)
					{
						float aspect_ratio = float(s_ShadowData->Resolution) / float(s_ShadowData->Resolution);
						projection = glm::perspective(glm::radians(s_ShadowData->LightFOV), aspect_ratio, s_ShadowData->DebugUBOBuffer.zNear, s_ShadowData->DebugUBOBuffer.zFar);
					}
					else
					{
						projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, s_ShadowData->DebugUBOBuffer.zNear, s_ShadowData->DebugUBOBuffer.zFar);
					}

					if (Engine::GetAPI() == RenderAPI::Vulkan)
					{
						projection[1][1] *= -1;
					}

					glm::mat4 view = glm::lookAt(
						s_ShadowData->LightPosition,
						s_ShadowData->LightDirection,
						glm::vec3(0.0f, 1.0f, 0.0f)
					);

					s_ShadowData->LightSpaceBuffer.LightSpaceMatrix = projection * view;
					s_ShadowData->LightSpaceUBO->SetData(&s_ShadowData->LightSpaceBuffer, sizeof(ShadowData::UBOData));

					s_ShadowData->DescriptorSet->SetUniformData("u_LightBuffer", s_ShadowData->LightSpaceUBO);
					s_ShadowData->DescriptorSet->Upload(commandBuffer);

					RendererCommand::BindDescriptorSets(commandBuffer, &s_ShadowData->DescriptorSet);

					registry.view<TransformComponent, ModelComponent>().each(
					[&](auto p_Entt, const TransformComponent& p_Transform, const ModelComponent& p_Model)
					{
						for (const auto& mesh : p_Model.ModelRef->GetMeshes())
						{
							auto transform = p_Transform.Transform.GetLocalMatrix();
							s_ShadowData->Shader->SetPushValue("Transform", &transform);

							s_ShadowData->Shader->BindPushConstants(commandBuffer);
							RendererCommand::DrawMesh(commandBuffer, mesh);
							s_RenderData->Stats.DrawCalls++;
						}
					});

					s_ShadowData->Pipeline->End(commandBuffer);
				}

				descriptorSet->SetUniformData("u_ShadowBuffer", s_ShadowData->LightSpaceUBO);
				descriptorSet->Upload(commandBuffer);

				descriptorSet->SetTexture("u_ShadowMap", s_ShadowData->ShadowMap);
				descriptorSet->Upload(commandBuffer);

				// shadow debug
				if (s_ShadowData->EnableDebugView)
				{
					s_ShadowData->DebugPipeline->Begin(commandBuffer);

					RendererCommand::SetViewport(0, 0, s_ShadowData->Resolution, s_ShadowData->Resolution);

					s_ShadowData->DebugUBO->SetData(&s_ShadowData->DebugUBOBuffer, sizeof(ShadowData::DebugUBOData));

					s_ShadowData->DebugDescriptorSet->SetUniformData("u_Buffer", s_ShadowData->DebugUBO);
					s_ShadowData->DebugDescriptorSet->Upload(commandBuffer);

					s_ShadowData->DebugDescriptorSet->SetTexture("u_ShadowMap", s_ShadowData->ShadowMap);
					s_ShadowData->DebugDescriptorSet->Upload(commandBuffer);

					RendererCommand::BindDescriptorSets(commandBuffer, &s_ShadowData->DebugDescriptorSet);
					RendererCommand::Draw(commandBuffer, nullptr, 6);
					s_RenderData->Stats.DrawCalls++;

					s_ShadowData->DebugPipeline->End(commandBuffer);
				}
			} // pbr


			pipeline->Begin(commandBuffer);

			RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

			RendererCommand::BindDescriptorSets(commandBuffer, &descriptorSet);

			registry.view<TransformComponent, ModelComponent>().each(
			[&] (auto p_Entt, const TransformComponent& p_Transform, const ModelComponent& p_Model)
			{
				for (const auto& mesh : p_Model.ModelRef->GetMeshes())
				{
					auto transform = p_Transform.Transform.GetLocalMatrix();
					shader->SetPushValue("Transform", &transform);

					mesh->BindMaterial(commandBuffer, shader, pbr);

					shader->BindPushConstants(commandBuffer);
					RendererCommand::DrawMesh(commandBuffer, mesh);
					s_RenderData->Stats.DrawCalls++;
				}
			});

			pipeline->End(commandBuffer);
		}

		if (s_RenderData->Settings.Renderer2D)
		{
			YM_PROFILE_SCOPE("Renderer2D")

			registry.view<TransformComponent, SpriteComponent, ShapeComponent>().each(
			[&](auto p_Entt, const TransformComponent& p_Transform, const SpriteComponent& p_Sprite, ShapeComponent& p_Shape)
			{
				size_t quadVertexCount = 4;
				const glm::vec2* texcoord;
				if (p_Sprite.Texture != nullptr)
					texcoord = p_Sprite.Texture->GetTexCoords();
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

				auto transform = p_Transform.Transform.GetLocalMatrix();

				float opacityThreshold = 0.98f;

				bool isTransparent = p_Sprite.Type == SurfaceType::Transparent || p_Sprite.Color.a < opacityThreshold;

				if (s_RenderData->Settings.Renderer2D_Quad && p_Shape.IsShape<Square>())
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
					if (p_Sprite.Texture != nullptr)
					{
						for (uint32_t i = 1; i < quadData->TextureSlotIndex; i++)
						{
							if (*quadData->TextureSlots[i].get() == *p_Sprite.Texture->GetTexture().get())
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
							quadData->TextureSlots[textureIndex] = p_Sprite.Texture->GetTexture();
							quadData->TextureSlotIndex++;
						}
					}

					for (int i = 0; i < quadVertexCount; i++)
					{
						QuadData::QuadVertex vertex{};
						vertex.Position = transform * quadData->VertexPositions[i];
						vertex.Color = p_Sprite.Color;
						vertex.TexCoord = texcoord[i];
						vertex.TexIndex = textureIndex;

						quadData->VertexBufferBase.push_back(vertex);
					}
					quadData->IndexCount += 6;
					s_RenderData->Stats.QuadCount++;
				}
				else if (p_Shape.IsShape<Circle>() && s_RenderData->Settings.Renderer2D_Circle)
				{
					Circle& circle = p_Shape.Get<Circle>();
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
					if (p_Sprite.Texture != nullptr)
					{
						for (uint32_t i = 1; i < circleData->TextureSlotIndex; i++)
						{
							if (*circleData->TextureSlots[i].get() == *p_Sprite.Texture->GetTexture().get())
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
							circleData->TextureSlots[textureIndex] = p_Sprite.Texture->GetTexture();
							circleData->TextureSlotIndex++;
						}
					}

					for (size_t i = 0; i < quadVertexCount; i++)
					{
						CircleData::CircleVertex vertex{};
						vertex.WorldPosition = transform * circleData->VertexPositions[i];
						vertex.LocalPosition = circleData->VertexPositions[i] * 2.0f;
						vertex.Color = p_Sprite.Color;
						vertex.Thickness = circle.Thickness;
						vertex.Fade = circle.Fade;
						vertex.TexIndex = textureIndex;

						circleData->VertexBufferBase.push_back(vertex);
					}
					circleData->IndexCount += 6;
					s_RenderData->Stats.CircleCount++;
				}
			});
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

	void Renderer::OnImgui()
	{
		ImGui::Begin("Renderer Settings");
		{
			static std::string outPath = "Resources/Screenshot/screenshot.png";
			ImGui::Text("OutPath: ");
			ImGui::SameLine();
			ImGui::InputText("##OutPathInput", outPath.data(), outPath.size());

			if (ImGui::Button("Screenshot"))
			{
				RendererCommand::SaveScreenshot(outPath, s_RenderData->MainTexture);
				//RendererCommand::SaveScreenshot(outPath, VulkanSwapchain::Get().GetBuffers()[VulkanSwapchain::Get().GetCurrentBuffer()]);
			}

			std::string quality[] = {
				"Low",
				"Medium",
				"High"
			};

			std::string projections[] =
			{
				"Perspective",
				"Orthographic"
			};


			if (s_ShadowData)
			{
				if (ImGui::BeginTable("##SettingsTable", 2, false))
				{
					ImGui::TableSetupColumn("ShadowMap", ImGuiTableColumnFlags_WidthFixed, 120.0f);
					ImGui::TableHeadersRow();
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);

					std::string curShadowQuality = quality[(int)s_RenderData->Settings.ShadowMap];
					ImGui::Text("Quality: ");

					ImGui::TableSetColumnIndex(1);

					if (ImGui::BeginCombo("##ShadowQuality", curShadowQuality.c_str()))
					{
						for (int i = 0; i < 3; i++)
						{
							bool isSelected = curShadowQuality == quality[i];

							if (ImGui::Selectable(quality[i].c_str(), isSelected))
							{
								curShadowQuality = quality[i];
								s_RenderData->Settings.ShadowMap = (Quality)i;
							}

							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}

						ImGui::EndCombo();
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					std::string curShadowProj = projections[s_ShadowData->DebugUBOBuffer.Mode];
					ImGui::Text("Projection: ");

					ImGui::TableSetColumnIndex(1);

					if (ImGui::BeginCombo("##ShadowProjection", curShadowProj.c_str()))
					{
						for (int i = 0; i < 2; i++)
						{
							bool isSelected = curShadowProj == projections[i];

							if (ImGui::Selectable(projections[i].c_str(), isSelected))
							{
								curShadowProj = projections[i];
								s_ShadowData->DebugUBOBuffer.Mode = i;
							}

							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}

						ImGui::EndCombo();
					}

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text("FOV: ");

					ImGui::TableSetColumnIndex(1);

					ImGui::InputFloat("##FOV", &s_ShadowData->LightFOV);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text("zNear: ");

					ImGui::TableSetColumnIndex(1);

					ImGui::InputFloat("##zNear", &s_ShadowData->DebugUBOBuffer.zNear);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text("zFar: ");

					ImGui::TableSetColumnIndex(1);

					ImGui::InputFloat("##zFar", &s_ShadowData->DebugUBOBuffer.zFar);
					
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text("Light Position: ");

					ImGui::TableSetColumnIndex(1);

					ImGui::DragFloat3("##ShadowMapLightPosition", glm::value_ptr(s_ShadowData->LightPosition), 0.1f);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text("Light Direction: ");

					ImGui::TableSetColumnIndex(1);

					ImGui::DragFloat3("##ShadowMapLightDirection", glm::value_ptr(s_ShadowData->LightDirection), 0.1f);

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					ImGui::Text("Debug View: ");

					ImGui::TableSetColumnIndex(1);

					ImGui::Checkbox("##ShadowMapDebugView", &s_ShadowData->EnableDebugView);

					ImGui::EndTable();
				}
			}
		}
		ImGui::End();

		if (s_ShadowData)
		{
			if (s_ShadowData->EnableDebugView)
			{
				float w		= (float)s_ShadowData->Resolution;
				float h		= (float)s_ShadowData->Resolution;
				auto imgui  = Application::Get().GetImGuiLayer();
				auto flags  = ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar;

				ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
				ImGui::Begin("ShadowMap DebugView", (bool*)0, flags);
				{
					ImGui::Image(imgui->AddTexture(s_ShadowData->DebugTexture), { w, h });
				}
				ImGui::End();
				ImGui::PopStyleColor();
			}

		}
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

		auto commandBuffer = Application::Get().GetWindow().GetContext()->GetCurrentCommandBuffer();

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

		s_RenderData->FinalPassPipeline->Begin(commandBuffer);

		RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

		s_RenderData->FinalPassDescriptorSet->SetTexture("u_Texture", s_RenderData->MainTexture);
		s_RenderData->FinalPassDescriptorSet->Upload(commandBuffer);

		RendererCommand::BindDescriptorSets(commandBuffer, &s_RenderData->FinalPassDescriptorSet);


		RendererCommand::Draw(commandBuffer, nullptr, 6);
		s_RenderData->Stats.DrawCalls++;

		s_RenderData->FinalPassPipeline->End(commandBuffer);
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

		VertexBuffer = VertexBuffer::Create(nullptr, MaxQuadVertices * sizeof(QuadVertex));

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
			auto commandBuffer = Application::Get().GetWindow().GetContext()->GetCurrentCommandBuffer();
			Pipeline->Begin(commandBuffer);

			RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

			DescriptorSets[0]->SetUniformData("u_Camera", s_RenderData->CameraUniformBuffer);
			DescriptorSets[0]->Upload(commandBuffer);
			DescriptorSets[1]->SetTexture("u_Textures", (const Ref<Texture>*)TextureSlots.data(), (uint32_t)TextureSlots.size());
			DescriptorSets[1]->Upload(commandBuffer);


			VertexBuffer->SetData(VertexBufferBase.data(), VertexBufferBase.size() * sizeof(QuadVertex));

			RendererCommand::BindDescriptorSets(commandBuffer, DescriptorSets.data(), (uint32_t)DescriptorSets.size());
			RendererCommand::DrawIndexed(commandBuffer, VertexBuffer, IndexBuffer);
			s_RenderData->Stats.DrawCalls++;

			Pipeline->End(commandBuffer);
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

		VertexBuffer = VertexBuffer::Create(nullptr, MaxCircleVertices * sizeof(CircleVertex));

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
			auto commandBuffer = Application::Get().GetWindow().GetContext()->GetCurrentCommandBuffer();
			Pipeline->Begin(commandBuffer);

			RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

			DescriptorSets[0]->SetUniformData("u_Camera", s_RenderData->CameraUniformBuffer);
			DescriptorSets[0]->Upload(commandBuffer);

			DescriptorSets[1]->SetTexture("u_Textures", (const Ref<Texture>*)TextureSlots.data(), (uint32_t)TextureSlots.size());
			DescriptorSets[1]->Upload(commandBuffer);

			VertexBuffer->SetData(VertexBufferBase.data(), VertexBufferBase.size() * sizeof(CircleVertex));

			RendererCommand::BindDescriptorSets(commandBuffer, DescriptorSets.data(), (uint32_t)DescriptorSets.size());
			RendererCommand::DrawIndexed(commandBuffer, VertexBuffer, IndexBuffer);
			s_RenderData->Stats.DrawCalls++;

			Pipeline->End(commandBuffer);
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

		auto commandBuffer = Application::Get().GetWindow().GetContext()->GetCurrentCommandBuffer();
		CompositePipeline->Begin(commandBuffer);

		RendererCommand::SetViewport(0, 0, s_RenderData->Width, s_RenderData->Height);

		CompositeDescriptorSet->SetTexture("u_AccumTexture", AccumTexture);
		CompositeDescriptorSet->SetTexture("u_RevealTexture", RevealTexture);
		CompositeDescriptorSet->SetTexture("u_OpaqueTexture", OpaqueTexture);
		CompositeDescriptorSet->Upload(commandBuffer);

		RendererCommand::BindDescriptorSets(commandBuffer, &CompositeDescriptorSet);
		RendererCommand::Draw(commandBuffer, nullptr, 6);
		s_RenderData->Stats.DrawCalls++;

		CompositePipeline->End(commandBuffer);
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
			pci   = p_PCI;
		}
		else
		{
			pci.ColorTargets[0]		= s_RenderData->MainTexture;
			pci.BlendModes[0]		= BlendMode::SrcAlphaOneMinusSrcAlpha;
			pci.TransparencyEnabled = true;
			pci.PolygonMode			= PolygonMode::FILL;
			pci.ClearTargets		= false;
			pci.SwapchainTarget		= false;
			pci.DepthTarget			= s_RenderData->DepthTexture;
			pci.DepthTest			= true;
			pci.DepthWrite			= true;
			pci.DebugName			= "ForwardPBRPipeline";
		}

		pci.Shader = Shader;

		Pipeline   = Pipeline::Get(pci);

		LightBuffer.NumLights = 0;
		for (int i = 0; i < MAX_LIGHTS; i++)
		{
			LightBuffer.Lights[i].Position			= glm::vec4(0.0f);
			LightBuffer.Lights[i].Color				= glm::vec4(1.0f);
			LightBuffer.Lights[i].Direction			= glm::vec4(0.0f);
			LightBuffer.Lights[i].AttenuationProps  = glm::vec3(1.0f, 0.09f, 0.032f);
			LightBuffer.Lights[i].Type				= 0.0f;
		}
	}

	void ShadowData::Init()
	{
		DebugShader			= Shader::Create("assets/shaders/shadow_debug_shader.glsl");
		DebugDescriptorSet  = DescriptorSet::Create({ /* Set */ 0, DebugShader });
		DebugUBO			= UniformBuffer::Create(sizeof(DebugUBOData));

		Shader				= Shader::Create("assets/shaders/shadow_shader.glsl");
		Shader->SetLayout({
			{ DataType::Float3, "a_Position" },
			{ DataType::Float3, "a_Normal"   },
			{ DataType::Float2, "a_TexCoord" },
			{ DataType::Float4, "a_Color"	 },
		});

		DescriptorSet		= DescriptorSet::Create({ /* Set */ 0, Shader });
		LightSpaceUBO		= UniformBuffer::Create(sizeof(UBOData));
	}

	void ShadowData::Begin()
	{
		Resolution		   = 4096;
		switch (s_RenderData->Settings.ShadowMap)
		{
			case Quality::Low:
				Resolution = 1024; break;
			case Quality::Medium:
				Resolution = 2048; break;
			case Quality::High:
				Resolution = 4096; break;
		}

		TextureSpecification spec = {};
		spec.Usage				  = TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT;
		spec.Format				  = TextureFormat::D32_FLOAT;
		spec.Width				  = Resolution;
		spec.Height				  = Resolution;
		spec.AnisotropyEnable	  = false;
		spec.GenerateMips		  = false;
		spec.MinFilter			  = TextureFilter::NEAREST;
		spec.MagFilter			  = TextureFilter::NEAREST;
		spec.WrapU				  = TextureWrap::CLAMP_TO_BORDER;
		spec.WrapV				  = TextureWrap::CLAMP_TO_BORDER;
		spec.BorderColorFlag	  = TextureBorderColor::OPAQUE_WHITE_FLOAT;
		spec.DebugName			  = "ShadowMap";

		ShadowMap				  = Texture2D::Get(spec);

		spec.Usage				  = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		spec.Format				  = TextureFormat::RGBA32_FLOAT;
		spec.MinFilter			  = TextureFilter::LINEAR;
		spec.MagFilter			  = TextureFilter::LINEAR;
		spec.DebugName			  = "ShadowMapDebug";

		DebugTexture			  = Texture2D::Get(spec);

		RendererCommand::ClearRenderTarget(ShadowMap);
		RendererCommand::ClearRenderTarget(DebugTexture, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));


		PipelineCreateInfo pci	  = {};
		pci.Shader				  = DebugShader;
		pci.ColorTargets[0]		  = DebugTexture;
		pci.TransparencyEnabled	  = false;
		pci.PolygonMode			  = PolygonMode::FILL;
		pci.ClearTargets		  = false;
		pci.SwapchainTarget		  = false;
		pci.DepthTest			  = false;
		pci.DepthWrite			  = false;
		pci.DebugName			  = "ShadowMapDebugPipeline";

		DebugPipeline			  = Pipeline::Get(pci);

		pci.FrontFace			  = FrontFace::COUNTER_CLOCKWISE;
		pci.CullMode			  = CullMode::FRONT;
		pci.Shader				  = Shader;
		pci.ColorTargets[0]		  = nullptr;
		pci.DepthTarget			  = ShadowMap;
		pci.DepthTest			  = true;
		pci.DepthWrite			  = true;
		pci.DepthBiasEnabled	  = true;
		pci.ConstantFactor		  = 1.25f;
		pci.SlopeFactor			  = 1.75f;
		pci.DebugName			  = "ShadowMapPipeline";

		Pipeline				  = Pipeline::Get(pci);


		LightSpaceBuffer.LightSpaceMatrix = glm::mat4(1.0f);
	}

	void SkyboxData::Init()
	{
		Shader		  = Shader::Create("assets/shaders/skybox_shader.glsl");

		DescriptorSet = DescriptorSet::Create({ /* Set */ 0, Shader });

		SkyboxUBO	  = UniformBuffer::Create(sizeof(BufferData));


		TextureArraySpecification spec   = {};

		std::array<uint8_t, 6 * 4> textureData = {};
		for (int i = 0; i < 6 * 4; i += 4)
		{
			textureData[i + 0] = 0;
			textureData[i + 1] = 0;
			textureData[i + 2] = 0;
			textureData[i + 3] = 255;
		}
		Skybox = TextureArray::Create(spec, (const uint8_t*)textureData.data(), textureData.size());
	}

	void SkyboxData::Begin(const glm::mat4& p_Projection, const glm::mat4& p_View)
	{
		PipelineCreateInfo pci  = {};
		pci.Shader				= Shader;
		pci.ColorTargets[0]		= s_RenderData->MainTexture;
		pci.DepthTarget			= s_RenderData->DepthTexture;
		pci.TransparencyEnabled = false;
		pci.PolygonMode			= PolygonMode::FILL;
		pci.ClearTargets		= false;
		pci.SwapchainTarget		= false;
		pci.DepthTest			= true;
		pci.DepthWrite			= false;
		pci.DebugName			= "SkyboxPipeline";

		Pipeline				= Pipeline::Get(pci);

		Buffer.Projection		= p_Projection;
		Buffer.View				= glm::mat4(glm::mat3(p_View));
		SkyboxUBO->SetData(&Buffer, sizeof(BufferData));
	}
}