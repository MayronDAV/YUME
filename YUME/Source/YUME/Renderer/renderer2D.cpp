#include "YUME/yumepch.h"
#include "renderer2D.h"
#include "shader.h"
#include "descriptor_set.h"
#include "renderer_command.h"
#include "YUME/Core/application.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace YUME
{
	#pragma region Render Structs

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		int TexIndex = 0;
	};

	#pragma endregion

	struct Renderer2Ddata
	{
		// ----------------------------
		// QUAD stuff

		static const uint32_t MaxQuads = 5000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint8_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> QuadShader;
		Ref<Pipeline> QuadPipeline;
		Ref<DescriptorSet> QuadDescriptorSet;

		uint32_t QuadIndexCount = 0;
		std::vector<QuadVertex> QuadVertexBufferBase;

		std::array<glm::vec4, 4> QuadVertexPositions;

		// ----------------------------
		// OTHER stuff

		Ref<Texture2D> WhiteTexture;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer{ glm::mat4(1.0f) };
		Ref<UniformBuffer> CameraUniformBuffer = nullptr;

		PolygonMode PolygonMode = PolygonMode::FILL;

		Renderer2D::Statistics Stats;

		// ----------------------------
	};

	static Renderer2Ddata* s_RenderData = nullptr;


	void Renderer2D::Init()
	{
		YM_PROFILE_FUNCTION()

		s_RenderData = new Renderer2Ddata();

		// ----------------------------
		// QUAD stuff

		s_RenderData->QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
	
		PipelineCreateInfo pci{};
		pci.Shader = s_RenderData->QuadShader;
		pci.BlendMode = BlendMode::SrcAlphaOneMinusSrcAlpha;
		pci.TransparencyEnabled = true;
		s_RenderData->QuadPipeline = Pipeline::Create(pci);

		s_RenderData->QuadShader->SetPipeline(s_RenderData->QuadPipeline);

		s_RenderData->QuadVertexArray = VertexArray::Create();

		s_RenderData->QuadVertexBuffer = VertexBuffer::Create(nullptr, Renderer2Ddata::MaxVertices * sizeof(QuadVertex), BufferUsage::DYNAMIC);
		s_RenderData->QuadVertexBuffer->SetLayout({
			{ DataType::Float3, "a_Position" },
			{ DataType::Float4, "a_Color"	 },
			{ DataType::Float2, "a_TexCoord" },
			{ DataType::Int , "a_TexIndex"   },
		});
		s_RenderData->QuadVertexArray->AddVertexBuffer(s_RenderData->QuadVertexBuffer);

		uint32_t* quadIndices = new uint32_t[Renderer2Ddata::MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < Renderer2Ddata::MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 0;
			quadIndices[i + 4] = offset + 2;
			quadIndices[i + 5] = offset + 3;

			offset += 4;
		}

		Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, Renderer2Ddata::MaxIndices);
		s_RenderData->QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;
	
		s_RenderData->QuadShader->AddVertexArray(s_RenderData->QuadVertexArray);

		s_RenderData->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		s_RenderData->QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		s_RenderData->QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		s_RenderData->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		s_RenderData->QuadDescriptorSet = DescriptorSet::Create(s_RenderData->QuadShader);

		// ----------------------------
		// OTHER stuff

		uint8_t whiteData[] = { 255, 255, 255, 255 };
		s_RenderData->WhiteTexture = Texture2D::Create({}, whiteData, sizeof(whiteData));

		s_RenderData->TextureSlots.fill(s_RenderData->WhiteTexture);

		s_RenderData->CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2Ddata::CameraData));
	}

	void Renderer2D::BeginScene(const Camera& p_Camera, const glm::mat4& p_Transform)
	{
		YM_PROFILE_FUNCTION()

		s_RenderData->QuadPipeline->SetPolygonMode(s_RenderData->PolygonMode);

		s_RenderData->CameraBuffer.ViewProjection = p_Camera.GetProjection() * glm::inverse(p_Transform);
		s_RenderData->CameraUniformBuffer->SetData(&s_RenderData->CameraBuffer, sizeof(Renderer2Ddata::CameraData));

		RendererCommand::Begin();

		StartBatch();
		ResetStats();
	}

	void Renderer2D::EndScene()
	{
		YM_PROFILE_FUNCTION()

		Flush();

		RendererCommand::End();
	}
	
	void Renderer2D::StartBatch()
	{
		YM_PROFILE_FUNCTION()

		s_RenderData->QuadIndexCount = 0;
		s_RenderData->TextureSlotIndex = 1;

		s_RenderData->QuadVertexBufferBase.clear();
	}

	void Renderer2D::Flush()
	{
		YM_PROFILE_FUNCTION()

		if (s_RenderData->QuadIndexCount)
		{
			YM_PROFILE_SCOPE("QuadFlush")

			s_RenderData->QuadShader->Bind();
			
			s_RenderData->QuadDescriptorSet->Bind(0);
			s_RenderData->QuadDescriptorSet->UploadUniform(0, s_RenderData->CameraUniformBuffer);

			s_RenderData->QuadDescriptorSet->Bind(1);
			s_RenderData->QuadDescriptorSet->UploadTexture2D(0, s_RenderData->TextureSlots.data(), (uint32_t)s_RenderData->TextureSlots.size());

			s_RenderData->QuadVertexBuffer->SetData(s_RenderData->QuadVertexBufferBase.data(), s_RenderData->QuadVertexBufferBase.size() * sizeof(QuadVertex));
			RendererCommand::DrawIndexed(s_RenderData->QuadVertexArray, s_RenderData->QuadIndexCount);

			s_RenderData->Stats.DrawCalls++;
		}
	}

	void Renderer2D::FlushAndReset()
	{
		YM_PROFILE_FUNCTION()

		Flush();

		StartBatch();
	}

	void Renderer2D::Shutdown()
	{
		YM_PROFILE_FUNCTION()

		if (s_RenderData)
		{
			YM_CORE_TRACE("Destroying render 2d data...")
			delete s_RenderData;
		}
	}

	void Renderer2D::SetPolygonMode(PolygonMode p_Mode)
	{
		s_RenderData->PolygonMode = p_Mode;
	}

	void Renderer2D::DrawQuad(const glm::vec3& p_Position, const glm::vec2& p_Size, const glm::vec4& p_Color, const Ref<Texture2D>& p_Texture)
	{
		YM_PROFILE_FUNCTION()

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 QuadTexCoords[] = {
			{0.0f, 0.0f},
			{1.0f, 0.0f},
			{1.0f, 1.0f},
			{0.0f, 1.0f}
		};

		if (s_RenderData->QuadIndexCount >= Renderer2Ddata::MaxIndices)
			FlushAndReset();

		int textureIndex = 0; // White texture
		if (p_Texture != nullptr)
		{
			for (uint32_t i = 1; i < s_RenderData->TextureSlotIndex; i++)
			{
				if (*s_RenderData->TextureSlots[i].get() == *p_Texture.get())
				{
					textureIndex = i;
					break;
				}
			}

			if (textureIndex == 0)
			{
				if (s_RenderData->TextureSlotIndex >= Renderer2Ddata::MaxTextureSlots)
					FlushAndReset();

				textureIndex = s_RenderData->TextureSlotIndex;
				s_RenderData->TextureSlots[s_RenderData->TextureSlotIndex] = p_Texture;
				s_RenderData->TextureSlotIndex++;
			}
		
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), p_Position);
		transform = glm::scale(transform, { p_Size.x, p_Size.y, 1.0f });

		YM_PROFILE_TAG("ID", s_RenderData->Stats.QuadCount)
		YM_PROFILE_TAG("QuadIndexCount", s_RenderData->QuadIndexCount)
		YM_PROFILE_TAG("Color", (float*)glm::value_ptr(p_Color))
		YM_PROFILE_TAG("TexIndex", textureIndex)

		for (int i = 0; i < quadVertexCount; i++)
		{
			QuadVertex vertex{};
			vertex.Position = transform * s_RenderData->QuadVertexPositions[i];
			vertex.Color = p_Color;
			vertex.TexCoord = QuadTexCoords[i];
			vertex.TexIndex = textureIndex;

			YM_PROFILE_TAG("vertex.Position", glm::value_ptr(vertex.Position))
			YM_PROFILE_TAG("vertex.TexIndex", glm::value_ptr(vertex.TexCoord))
			
			s_RenderData->QuadVertexBufferBase.push_back(vertex);
		}

		s_RenderData->QuadIndexCount += 6;
		s_RenderData->Stats.QuadCount++;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_RenderData->Stats;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_RenderData->Stats, 0, sizeof(Statistics));
	}
}
