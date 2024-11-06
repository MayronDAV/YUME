#include "YUME/yumepch.h"
#include "model.h"
#include "texture_importer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny/tiny_obj_loader.h>




namespace YUME
{
	Model::Model(const std::string& p_Path, bool p_FlipYTexCoord)
	{
		LoadModel(p_Path, p_FlipYTexCoord);
	}

	void Model::LoadModel(const std::string& p_Path, bool p_FlipYTexCoord)
	{
		YM_PROFILE_FUNCTION()

		m_Meshes.clear();
		std::filesystem::path path = p_Path;

		if (path.extension() == ".obj")
		{
			LoadModelOBJ(p_Path, p_FlipYTexCoord);
		}
		else
		{
			YM_CORE_ERROR("Unsupported model extension!")
			return;
		}

		m_Path = p_Path;
		YM_CORE_INFO("Loaded Model: {}", p_Path)
	}

	Ref<Texture2D> Model::LoadMaterialTexture(const std::string& p_Path)
	{
		auto it = m_TexturesLoaded.find(p_Path);
		if (it != m_TexturesLoaded.end())
		{
			return m_TexturesLoaded[p_Path];
		}

		auto texture = TextureImporter::LoadTexture2D(p_Path);
		if (texture)
		{
			m_TexturesLoaded[p_Path] = texture;
		}

		return texture;
	}

	void Model::LoadModelOBJ(const std::string& p_Path, bool p_FlipYTexCoord)
	{
		YM_PROFILE_FUNCTION()

		std::string directory = std::filesystem::path(p_Path).parent_path().string() + "/";
		tinyobj::attrib_t attrib;
		std::string error = std::string();

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, p_Path.c_str(), directory.c_str()))
		{
			YM_CORE_ERROR("Failed to load OBJ: {}", error)
			return;
		}

		if (!error.empty())
		{
			YM_CORE_ERROR("[OBJ_LOG] - {}", error)
		}

		for (const auto& shape : shapes)
		{
			uint32_t vertexCount = 0;
			const uint32_t numIndices = static_cast<uint32_t>(shape.mesh.indices.size());
			const uint32_t numVertices = numIndices;
			std::vector<MeshVertex> vertices(numVertices);
			std::vector<uint32_t> indices(numIndices);

			float meshAlpha = 1.0f;

			for (uint32_t i = 0; i < shape.mesh.indices.size(); i++)
			{
				auto& index = shape.mesh.indices[i];
				MeshVertex vertex;

				if (!attrib.texcoords.empty())
				{
					float y = attrib.texcoords[2 * index.texcoord_index + 1];

					vertex.TexCoord = (glm::vec2(
						attrib.texcoords[2 * index.texcoord_index + 0],
						(p_FlipYTexCoord) ? 1.0f - y : y));
				}
				else
				{
					vertex.TexCoord = glm::vec2(0.0f, 0.0f);
				}
				vertex.Position = (glm::vec3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]));

				if (!attrib.normals.empty())
				{
					vertex.Normal = (glm::vec3(
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]));
				}

				glm::vec4 color = glm::vec4(1.0f);

				if (shape.mesh.material_ids[0] >= 0)
				{
					tinyobj::material_t* mp = &materials[shape.mesh.material_ids[0]];
					color = glm::vec4(mp->diffuse[0], mp->diffuse[1], mp->diffuse[2], 1.0f);
					if (mp->dissolve > 0.0f)
					{
						color.a = mp->dissolve;
					}
				}

				vertex.Color = color;
				meshAlpha = color.a;
				vertices[vertexCount] = vertex;
				indices[vertexCount] = vertexCount;

				vertexCount++;
			}

			if (attrib.normals.empty())
				Mesh::GenerateNormals(vertices.data(), vertexCount, indices.data(), numIndices);

			MaterialProperties properties;

			if (shape.mesh.material_ids[0] >= 0)
			{
				tinyobj::material_t* mp = &materials[shape.mesh.material_ids[0]];

				properties.Name = mp->name;

				if (meshAlpha <= 0.98f)
				{
					properties.Surface = SurfaceType::Transparent;
				}

				if (mp->alpha_texname.length() > 0)
				{
					properties.Surface = SurfaceType::Transparent;
					auto texture = LoadMaterialTexture(directory + mp->alpha_texname);
					if (texture)
						properties.Textures.OpacityMap = texture;
				}

				if (mp->diffuse_texname.length() > 0)
				{
					auto texture = LoadMaterialTexture(directory + mp->diffuse_texname);
					if (texture)
						properties.Textures.AlbedoMap = texture;
				}

				if (mp->bump_texname.length() > 0)
				{
					auto texture = LoadMaterialTexture(directory + mp->bump_texname);
					if (texture)
					{
						properties.NormalMap = true;
						properties.Textures.NormalMap = texture;
					}
				}

				if (mp->specular_highlight_texname.length() > 0)
				{
					auto texture = LoadMaterialTexture(directory + mp->specular_highlight_texname);
					if (texture)
					{
						properties.SpecularMap = true;
						properties.Textures.SpecularMap = texture;
					}
				}

				if (mp->ambient_texname.length() > 0)
				{
					auto texture = LoadMaterialTexture(directory + mp->ambient_texname);
					if (texture)
						properties.Textures.AoMap = texture;
				}

				if (mp->roughness_texname.length() > 0)
				{
					auto texture = LoadMaterialTexture(directory + mp->roughness_texname);
					if (texture)
						properties.Textures.RoughnessMap = texture;
				}

				if (mp->metallic_texname.length() > 0)
				{
					auto texture = LoadMaterialTexture(directory + mp->metallic_texname);
					if (texture)
						properties.Textures.MetallicMap = texture;
				}
			}

			auto mesh = CreateRef<Mesh>(indices, vertices);
			auto material = CreateRef<Material>();
			material->SetProperties(properties);
			mesh->SetMaterial(material);
			m_Meshes.push_back(mesh);
		}

		m_TexturesLoaded.clear();
	}

} // YUME
