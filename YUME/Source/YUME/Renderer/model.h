#pragma once

#include "YUME/Core/base.h"
#include "mesh.h"
#include "texture.h"



namespace YUME
{
	class YM_API Model : public Asset
	{
		ASSET_CLASS_TYPE(Model)

		public:
			Model() = default;
			Model(const std::string& p_Path, bool p_FlipYTexCoord = false);

			void LoadModel(const std::string& p_Path, bool p_FlipYTexCoord = false);

			void AddMesh(const Ref<Mesh>& p_Mesh) { m_Meshes.push_back(p_Mesh); }

			std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
			const std::vector<Ref<Mesh>>& GetMeshes() const { return m_Meshes; }

			void SetGPUInstance(bool p_Enable) { m_GPUInstance = p_Enable; }

			inline bool operator== (const Model& p_Rhs) const { return Handle == p_Rhs.Handle && m_Path == p_Rhs.m_Path && m_GPUInstance == m_GPUInstance; }

		private:
			Ref<Texture2D> LoadMaterialTexture(const std::string& p_Path, const TextureSpecification& p_Spec);

			void LoadModelOBJ(const std::string& p_Path, bool p_FlipYTexCoord);
			void LoadModelGLTF(const std::string& p_Path, bool p_FlipYTexCoord);

		private:
			std::unordered_map<std::string, Ref<Texture2D>> m_TexturesLoaded;

			std::string m_Path = std::string();
			bool m_GPUInstance = false;
			std::vector<Ref<Mesh>> m_Meshes;
	};


} // YUME