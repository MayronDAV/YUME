#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "texture.h"
#include "shader.h"
#include "descriptor_set.h"



namespace YUME
{
	
	struct PBRTextures
	{
		Ref<Texture2D> AlbedoMap     = nullptr;
		Ref<Texture2D> OpacityMap    = nullptr; // Maybe this isn't just fo pbr?
		Ref<Texture2D> NormalMap     = nullptr;
		Ref<Texture2D> SpecularMap   = nullptr;
		Ref<Texture2D> MetallicMap   = nullptr;
		Ref<Texture2D> RoughnessMap  = nullptr;
		Ref<Texture2D> AoMap		 = nullptr;
	};

	struct MaterialProperties
	{
		std::string	   Name			 = "None";

		SurfaceType	   Surface		 = SurfaceType::Opaque;
		bool		   SpecularMap	 = false;
		bool		   NormalMap	 = false;
		float		   AlphaCutOff	 = 0.1f;

		PBRTextures    Textures		 = {};
	};

	class YM_API Material
	{
		public:
			Material() = default;
			
			void SetProperties(const MaterialProperties& p_Properties) { m_Properties = p_Properties; }
			
			void Bind(const Ref<Shader>& p_Shader, bool p_PBR = false);

			static void CreateDefaultTextures();
			static void DestroyDefaultTextures();

		private:
			void UploadMaterialProperties();
			void CreateDescriptorSet(uint32_t p_Set, const Ref<Shader>& p_Shader, bool p_PBR);

			static Ref<Texture2D> s_DefaultAlbedoTexture;
			static Ref<Texture2D> s_DefaultNormalTexture;
			static Ref<Texture2D> s_DefaultSpecularTexture;
			static Ref<Texture2D> s_DefaultTexture;

		private:
			MaterialProperties m_Properties;
			Ref<DescriptorSet> m_DescriptorSet;
			bool m_TexturesUpdated = false;

	};


} // YUME