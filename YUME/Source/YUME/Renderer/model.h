#pragma once

#include "YUME/Core/base.h"
#include "mesh.h"
#include "texture.h"



namespace YUME
{
	class YM_API Model
	{
		public:
			Model() = default;
			Model(const std::string& p_Path, bool p_FlipYTexCoord = true);

			void LoadModel(const std::string& p_Path, bool p_FlipYTexCoord);

			std::vector<Ref<Mesh>>& GetMeshes() { return m_Meshes; }
			const std::vector<Ref<Mesh>>& GetMeshes() const { return m_Meshes; }

			void SetGPUInstance(bool p_Enable) { m_GPUInstance = p_Enable; }

			inline bool operator== (const Model& p_Rhs) const { return m_Path == p_Rhs.m_Path && m_GPUInstance == m_GPUInstance; }

		private:
			Ref<Texture2D> LoadMaterialTexture(const std::string& p_Path);

			void LoadModelOBJ(const std::string& p_Path, bool p_FlipYTexCoord);

		private:
			std::unordered_map<std::string, Ref<Texture2D>> m_TexturesLoaded;

			std::string m_Path = std::string();
			bool m_GPUInstance = false;
			std::vector<Ref<Mesh>> m_Meshes;
	};


} // YUME