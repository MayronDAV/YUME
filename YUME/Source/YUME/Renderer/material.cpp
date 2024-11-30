#include "YUME/yumepch.h"
#include "material.h"
#include "renderer_command.h"



namespace YUME
{
	Ref<Texture2D> Material::s_DefaultAlbedoTexture = nullptr;
	Ref<Texture2D> Material::s_DefaultNormalTexture = nullptr;
	Ref<Texture2D> Material::s_DefaultSpecularTexture = nullptr;
	Ref<Texture2D> Material::s_DefaultTexture = nullptr;


	void Material::CreateDefaultTextures()
	{
		uint8_t albData[] = { 255, 255, 255, 255 };
		s_DefaultAlbedoTexture = Texture2D::Create({}, albData, sizeof(albData));

		uint8_t normData[] = { 128, 128, 255, 255 };
		s_DefaultNormalTexture = Texture2D::Create({}, normData, sizeof(normData));

		uint8_t specData[] = { 0, 0, 0, 255 };
		s_DefaultSpecularTexture = Texture2D::Create({}, specData, sizeof(specData));

		uint8_t defData[] = { 128, 128, 128, 255 };
		s_DefaultTexture = Texture2D::Create({}, defData, sizeof(defData));
	}

	void Material::DestroyDefaultTextures()
	{
		if (s_DefaultAlbedoTexture)
			s_DefaultAlbedoTexture.reset();

		if (s_DefaultNormalTexture)
			s_DefaultNormalTexture.reset();

		if (s_DefaultSpecularTexture)
			s_DefaultSpecularTexture.reset();

		if (s_DefaultTexture)
			s_DefaultTexture.reset();
	}

	void Material::Bind(CommandBuffer* p_CommandBuffer, const Ref<Shader>& p_Shader, bool p_PBR)
	{
		if (!m_TexturesUpdated)
		{
			CreateDescriptorSet(1, p_Shader, p_PBR);
			m_TexturesUpdated = true;
		}

		RendererCommand::BindDescriptorSets(p_CommandBuffer, &m_DescriptorSet);
	}

	void Material::UploadMaterialProperties()
	{
		struct
		{
			glm::vec4 AlbedoColor;
			int		  SpecularMap;
			int		  NormalMap;
			float	  AlphaCutOff;

		} buffer{ m_Properties.AlbedoColor, (m_Properties.SpecularMap) ? 1 : 0, (m_Properties.NormalMap) ? 1 : 0, m_Properties.AlphaCutOff};

		m_DescriptorSet->SetUniformData("u_MaterialProperties", UniformBuffer::Create(&buffer, sizeof(buffer)));

		m_DescriptorSet->Upload();
	}

	void Material::CreateDescriptorSet(uint32_t p_Set, const Ref<Shader>& p_Shader, bool p_PBR)
	{
		if (m_DescriptorSet)
			m_DescriptorSet.reset();

		m_DescriptorSet = DescriptorSet::Create({ p_Set, p_Shader });

		std::string texName = "u_AlbedoTexture";
		if (!p_PBR)
			texName = "u_Texture";

		if (m_Properties.Textures.AlbedoMap)
		{
			m_DescriptorSet->SetTexture(texName, m_Properties.Textures.AlbedoMap);
		}
		else
		{
			m_DescriptorSet->SetTexture(texName, s_DefaultAlbedoTexture);
		}

		m_DescriptorSet->Upload();

		if (p_PBR)
		{
			if (m_Properties.Textures.NormalMap)
			{
				m_DescriptorSet->SetTexture("u_NormalTexture", m_Properties.Textures.NormalMap);
			}
			else
			{
				m_DescriptorSet->SetTexture("u_NormalTexture", s_DefaultNormalTexture);
			}

			if (m_Properties.Textures.SpecularMap)
			{
				m_DescriptorSet->SetTexture("u_SpecularTexture", m_Properties.Textures.SpecularMap);
			}
			else
			{
				m_DescriptorSet->SetTexture("u_SpecularTexture", s_DefaultSpecularTexture);
			}

			if (m_Properties.Textures.RoughnessMap)
			{
				m_DescriptorSet->SetTexture("u_RoughnessTexture", m_Properties.Textures.RoughnessMap);
			}
			else
			{
				m_DescriptorSet->SetTexture("u_RoughnessTexture", s_DefaultTexture);
			}

			if (m_Properties.Textures.MetallicMap)
			{
				m_DescriptorSet->SetTexture("u_MetallicTexture", m_Properties.Textures.MetallicMap);
			}
			else
			{
				m_DescriptorSet->SetTexture("u_MetallicTexture", s_DefaultTexture);
			}

			if (m_Properties.Textures.AoMap)
			{
				m_DescriptorSet->SetTexture("u_AoTexture", m_Properties.Textures.AoMap);
			}
			else
			{
				m_DescriptorSet->SetTexture("u_AoTexture", s_DefaultTexture);
			}

			m_DescriptorSet->Upload();

			UploadMaterialProperties();
		}
	}
}
