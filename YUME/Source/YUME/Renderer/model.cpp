#include "YUME/yumepch.h"
#include "model.h"
#include "texture_importer.h"
#include "YUME/Math/transform.h"
#include "YUME/Utils/utils.h"

#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

// OBJ
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny/tiny_obj_loader.h>

// GLTF
#include <json/json.hpp>
#include <stb_image_resize2.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
//#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tinygltf/tiny_gltf.h>
#include <glm/gtc/type_ptr.hpp>



#define OBJ_PREFIX  "[OBJ]    - "
#define GLTF_PREFIX "[GLTF]   - "


namespace YUME
{
	Model::Model(const std::string& p_Path, bool p_FlipYTexCoord)
	{
		YM_PROFILE_FUNCTION()

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
		else if (path.extension() == ".gltf" || path.extension() == ".glb")
		{
			LoadModelGLTF(p_Path, p_FlipYTexCoord);
		}
		else
		{
			YM_CORE_ERROR("Unsupported model extension!")
			return;
		}

		m_Path = p_Path;
		YM_CORE_INFO("Loaded Model: {}", p_Path)
	}

	Ref<Texture2D> Model::LoadMaterialTexture(const std::string& p_Path, const TextureSpecification& p_Spec)
	{
		YM_PROFILE_FUNCTION()

		auto it = m_TexturesLoaded.find(p_Path);
		if (it != m_TexturesLoaded.end())
		{
			return m_TexturesLoaded[p_Path];
		}

		auto texture = TextureImporter::LoadTexture2D(p_Path, p_Spec);
		if (texture)
		{
			m_TexturesLoaded[p_Path] = texture;
		}

		return texture;
	}

	static std::vector<std::string> SplitString(const std::string& p_Str, char p_Delimiter)
	{
		YM_PROFILE_FUNCTION()

		std::vector<std::string> result;
		std::stringstream ss(p_Str);
		std::string token;

		while (std::getline(ss, token, p_Delimiter))
		{
			result.push_back(token);
		}
		return result;
	}
	
	#pragma region OBJ_LOADER

	void Model::LoadModelOBJ(const std::string& p_Path, bool p_FlipYTexCoord)
	{
		YM_PROFILE_FUNCTION()

		std::string directory = std::filesystem::path(p_Path).parent_path().string() + "/";
		tinyobj::attrib_t attrib;
		std::string error = std::string();
		std::string warn = std::string();

		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, p_Path.c_str(), directory.c_str()))
		{
			auto strs = SplitString(error, '\n');
			for (const auto& str : strs)
			{
				if (!str.empty())
					YM_CORE_ERROR(OBJ_PREFIX "{}", str)
			}
			return;
		}

		if (!warn.empty())
		{
			auto strs = SplitString(warn, '\n');
			for (const auto& str : strs)
			{
				if (!str.empty())
					YM_CORE_WARN(OBJ_PREFIX "{}", str)
			}
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

				properties.AlbedoColor = glm::vec4(mp->diffuse[0], mp->diffuse[1], mp->diffuse[2], mp->dissolve);

				TextureSpecification spec{};
				spec.AnisotropyEnable = true;
				spec.GenerateMips	  = true;
				spec.MinFilter		  = TextureFilter::NEAREST;
				spec.MagFilter		  = TextureFilter::NEAREST;
				spec.DebugName		  = "OBJ - Material Texture";
				if (!mp->name.empty())
				{
					spec.DebugName	  = "OBJ - " + mp->name + " Texture";
				}

				if (meshAlpha <= 0.98f)
				{
					properties.Surface = SurfaceType::Transparent;
				}

				if (mp->alpha_texname.length() > 0)
				{
					properties.Surface = SurfaceType::Transparent;
					
					spec.WrapU = mp->alpha_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->alpha_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->alpha_texname, spec);
					if (texture)
						properties.Textures.OpacityMap = texture;
				}

				if (mp->diffuse_texname.length() > 0)
				{
					spec.WrapU = mp->diffuse_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->diffuse_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->diffuse_texname, spec);
					if (texture)
						properties.Textures.AlbedoMap = texture;
				}

				if (mp->bump_texname.length() > 0)
				{
					spec.WrapU = mp->bump_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->bump_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->bump_texname, spec);
					if (texture)
					{
						properties.NormalMap = true;
						properties.Textures.NormalMap = texture;
					}
				}

				if (mp->specular_highlight_texname.length() > 0)
				{
					spec.WrapU = mp->specular_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->specular_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->specular_highlight_texname, spec);
					if (texture)
					{
						properties.SpecularMap = true;
						properties.Textures.SpecularMap = texture;
					}
				}

				if (mp->ambient_texname.length() > 0)
				{
					spec.WrapU = mp->ambient_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->ambient_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->ambient_texname, spec);
					if (texture)
						properties.Textures.AoMap = texture;
				}

				if (mp->roughness_texname.length() > 0)
				{
					spec.WrapU = mp->roughness_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->roughness_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->roughness_texname, spec);
					if (texture)
						properties.Textures.RoughnessMap = texture;
				}

				if (mp->metallic_texname.length() > 0)
				{
					spec.WrapU = mp->metallic_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;
					spec.WrapV = mp->metallic_texopt.clamp ? TextureWrap::CLAMP_TO_EDGE : TextureWrap::REPEAT;

					auto texture = LoadMaterialTexture(directory + mp->metallic_texname, spec);
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


	#pragma endregion

	#pragma region GLTF_LOADER

	struct GLTFTexture
	{
		tinygltf::Image* Image;
		tinygltf::Sampler* Sampler;
	};

	static std::unordered_map<int, int> s_GLTF_COMPONENT_LENGTH_LOOKUP;
	static std::unordered_map<int, int> s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP;
	static bool s_MapsInitialised = false;

	static TextureWrap GetWrapMode(int p_Mode)
	{
		switch (p_Mode)
		{
			case TINYGLTF_TEXTURE_WRAP_REPEAT:			return TextureWrap::REPEAT;
			case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:	return TextureWrap::CLAMP_TO_EDGE;
			case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: return TextureWrap::MIRRORED_REPEAT;

			default:
				return TextureWrap::REPEAT;
		}
	}

	static TextureFilter GetFilter(int p_Value)
	{
		switch (p_Value)
		{
			case TINYGLTF_TEXTURE_FILTER_NEAREST:
			case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
			case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
				return TextureFilter::NEAREST;

			case TINYGLTF_TEXTURE_FILTER_LINEAR:
			case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
			case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
				return TextureFilter::LINEAR;

			default:
				return TextureFilter::LINEAR;
		}
	}

	static std::vector<Ref<Material>> LoadMaterials(tinygltf::Model& p_GLTFModel)
	{
		YM_PROFILE_FUNCTION()

		std::vector<Ref<Texture2D>> loadedTextures;
		std::vector<Ref<Material>> loadedMaterials;
		loadedTextures.reserve(p_GLTFModel.textures.size());
		loadedMaterials.reserve(p_GLTFModel.materials.size());

		for (tinygltf::Texture& gltfTexture : p_GLTFModel.textures)
		{
			GLTFTexture imageAndSampler{};

			if (gltfTexture.source != -1)
			{
				imageAndSampler.Image		= &p_GLTFModel.images.at(gltfTexture.source);
			}

			if (gltfTexture.sampler != -1)
			{
				imageAndSampler.Sampler		= &p_GLTFModel.samplers.at(gltfTexture.sampler);
			}

			if (imageAndSampler.Image)
			{
				int texWidth				= imageAndSampler.Image->width;
				int texHeight				= imageAndSampler.Image->height;

				TextureSpecification spec	= {};
				spec.Usage					= TextureUsage::TEXTURE_SAMPLED;
				spec.Format					= TextureFormat::RGBA8_SRGB;
				spec.AnisotropyEnable		= true;
				spec.GenerateMips			= true;
				spec.MinFilter				= TextureFilter::NEAREST;
				spec.MagFilter				= TextureFilter::NEAREST;
				spec.DebugName				= "GLTF - Material Texture";


				if (gltfTexture.sampler != -1)
				{
					if (!imageAndSampler.Sampler->name.empty())
					{
						spec.DebugName		= "GLTF - " + imageAndSampler.Sampler->name;
					}

					spec.MinFilter			= GetFilter(imageAndSampler.Sampler->minFilter);
					spec.MagFilter			= GetFilter(imageAndSampler.Sampler->minFilter);
					spec.WrapU				= GetWrapMode(imageAndSampler.Sampler->wrapS);
					spec.WrapV				= GetWrapMode(imageAndSampler.Sampler->wrapT);
				}
				else
				{
					YM_CORE_WARN(GLTF_PREFIX "MISSING SAMPLER");
				}

				uint8_t* pixels				= imageAndSampler.Image->image.data();

				uint32_t maxWidth			= 2048;
				uint32_t maxHeight			= 2048;
				Utils::GetMaxImagesSize(&maxWidth, &maxHeight);

				bool freeData				= false;

				if (maxWidth > 0 && maxHeight > 0 && (uint32_t(texWidth) > maxWidth || uint32_t(texHeight) > maxHeight))
				{
					int texWidthOld			= imageAndSampler.Image->width;
					int texHeightOld		= imageAndSampler.Image->height;

					float aspectRatio		= static_cast<float>(texWidth) / static_cast<float>(texHeight);
					if (uint32_t(texWidth) > maxWidth)
					{
						texWidth			= maxWidth;
						texHeight			= static_cast<uint32_t>(float(maxWidth) / aspectRatio);
					}
					if (uint32_t(texHeight) > maxHeight)
					{
						texHeight			= maxHeight;
						texWidth			= static_cast<uint32_t>(float(maxHeight) * aspectRatio);
					}

					// Resize the image using stbir (a simple image resizing library)
					int resizedChannels		= STBIR_RGBA; // RGBA format
					stbi_uc* resizedPixels  = (stbi_uc*)malloc(texWidth * texHeight * resizedChannels);
					stbir_resize_uint8_linear(pixels, texWidthOld, texHeightOld, 0, resizedPixels, texWidth, texHeight, 0, STBIR_RGBA);

					pixels					= resizedPixels;
					freeData				= true;
				}

				spec.Width					= (uint32_t)texWidth;
				spec.Height					= (uint32_t)texHeight;
				Ref<Texture2D> texture2D	= Texture2D::Create(spec, pixels, texWidth * texHeight * STBIR_RGBA);
				if (texture2D)
				{
					loadedTextures.push_back(texture2D);
				}
				else
				{
					YM_CORE_ERROR(GLTF_PREFIX "Failed to create texture!");
				}

				if (freeData)
					free(pixels);

				imageAndSampler.Image->image.clear();
				imageAndSampler.Image->image.shrink_to_fit();
			}
		}

		auto TextureName = [&](int p_Index)
		{
			if (p_Index >= 0)
			{
				const tinygltf::Texture& tex = p_GLTFModel.textures[p_Index];
				if (tex.source >= 0 && tex.source < loadedTextures.size())
				{
					return loadedTextures[tex.source];
				}
			}
			return Ref<Texture2D>();
		};

		for (tinygltf::Material& mat : p_GLTFModel.materials)
		{

			Ref<Material> pbrMaterial = CreateRef<Material>();
			MaterialProperties properties;

			const tinygltf::PbrMetallicRoughness& pbr = mat.pbrMetallicRoughness;
			properties.Textures.AlbedoMap			  = TextureName(pbr.baseColorTexture.index);
			properties.Textures.NormalMap			  = TextureName(mat.normalTexture.index);
			properties.Textures.AoMap				  = TextureName(mat.occlusionTexture.index);
			properties.Textures.MetallicMap			  = TextureName(pbr.metallicRoughnessTexture.index);

			if (properties.Textures.NormalMap)
			{
				properties.NormalMap				  = true;
			}

			auto baseColorFactor					  = mat.values.find("baseColorFactor");

			if (baseColorFactor != mat.values.end())
			{
				properties.AlbedoColor = glm::vec4(
					(float)baseColorFactor->second.ColorFactor()[0],
					(float)baseColorFactor->second.ColorFactor()[1],
					(float)baseColorFactor->second.ColorFactor()[2],
					1.0f
				);

				if (baseColorFactor->second.ColorFactor().size() > 3)
					properties.AlbedoColor.w = (float)baseColorFactor->second.ColorFactor()[3];
			}

			// Extensions
			auto metallicGlossinessWorkflow = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
			if (metallicGlossinessWorkflow != mat.extensions.end())
			{
				if (metallicGlossinessWorkflow->second.Has("diffuseTexture"))
				{
					int index = metallicGlossinessWorkflow->second.Get("diffuseTexture").Get("index").Get<int>();
					properties.Textures.AlbedoMap = loadedTextures[p_GLTFModel.textures[index].source];
				}

				if (metallicGlossinessWorkflow->second.Has("metallicGlossinessTexture"))
				{
					int index = metallicGlossinessWorkflow->second.Get("metallicGlossinessTexture").Get("index").Get<int>();
					properties.Textures.RoughnessMap = loadedTextures[p_GLTFModel.textures[index].source];
				}

				if (metallicGlossinessWorkflow->second.Has("diffuseFactor"))
				{
					auto& factor = metallicGlossinessWorkflow->second.Get("diffuseFactor");
					properties.AlbedoColor.x = factor.ArrayLen() > 0 ? factor.Get(0).IsNumber() ? (float)factor.Get(0).Get<double>() : (float)factor.Get(0).Get<int>() : 1.0f;
					properties.AlbedoColor.y = factor.ArrayLen() > 1 ? factor.Get(1).IsNumber() ? (float)factor.Get(1).Get<double>() : (float)factor.Get(1).Get<int>() : 1.0f;
					properties.AlbedoColor.z = factor.ArrayLen() > 2 ? factor.Get(2).IsNumber() ? (float)factor.Get(2).Get<double>() : (float)factor.Get(2).Get<int>() : 1.0f;
					properties.AlbedoColor.w = factor.ArrayLen() > 3 ? factor.Get(3).IsNumber() ? (float)factor.Get(3).Get<double>() : (float)factor.Get(3).Get<int>() : 1.0f;
				}
			}

			//YM_CORE_INFO("Name: {} - AlbedoMap: {}, NormalMap: {}, AoMap: {}, MetallicMap: {}",
			//	mat.name,
			//	properties.Textures.AlbedoMap	? "Yes" : "No",
			//	properties.Textures.NormalMap	? "Yes" : "No",
			//	properties.Textures.AoMap		? "Yes" : "No",
			//	properties.Textures.MetallicMap ? "Yes" : "No"
			//)

			if (mat.alphaCutoff != 0.5f)
				properties.AlphaCutOff = (float)mat.alphaCutoff;

			if (mat.alphaMode != "OPAQUE")
				properties.Surface = SurfaceType::Transparent;

			properties.Name = mat.name;

			pbrMaterial->SetProperties(properties);


			loadedMaterials.push_back(pbrMaterial);
		}

		return loadedMaterials;
	}

	static std::vector<Mesh*> LoadMesh(tinygltf::Model& p_Model, tinygltf::Mesh& p_Mesh, std::vector<Ref<Material>>& p_Materials, Math::Transform& p_ParentTransform, bool p_FlipYTexCoord)
	{
		std::vector<Mesh*> meshes;

		for (auto& primitive : p_Mesh.primitives)
		{
			std::vector<MeshVertex> vertices;

			uint32_t vertexCount = (uint32_t)(primitive.attributes.empty() ? 0 : p_Model.accessors.at(primitive.attributes["POSITION"]).count);

			bool hasNormals		 = false;

			if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
				hasNormals		 = true;

			vertices.resize(vertexCount);

			for (auto& attribute : primitive.attributes)
			{
				auto& accessor	 = p_Model.accessors.at(attribute.second);
				auto& bufferView = p_Model.bufferViews.at(accessor.bufferView);
				auto& buffer	 = p_Model.buffers.at(bufferView.buffer);

				int componentLength		  = s_GLTF_COMPONENT_LENGTH_LOOKUP[accessor.type];
				int componentTypeByteSize = s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP[accessor.componentType];

				int stride				  = accessor.ByteStride(bufferView);

				// Extra vertex data from buffer
				size_t bufferOffset		  = bufferView.byteOffset + accessor.byteOffset;
				int bufferLength		  = static_cast<int>(accessor.count) * componentLength * componentTypeByteSize;
				std::vector<uint8_t> data;
				data.resize(bufferLength);
				uint8_t* arrayData = data.data();
				memcpy(arrayData, buffer.data.data() + bufferOffset, bufferLength);

				// -------- Position attribute -----------

				if (attribute.first == "POSITION")
				{
					size_t positionCount = accessor.count;
					glm::vec3* positions = reinterpret_cast<glm::vec3*>(data.data());
					for (auto p = 0; p < positionCount; ++p)
					{
						vertices[p].Position = p_ParentTransform.GetWorldMatrix() * glm::vec4(positions[p], 1.0f);
						YM_CORE_ASSERT(
							!glm::isinf(vertices[p].Position.x) && !glm::isinf(vertices[p].Position.y) && !glm::isinf(vertices[p].Position.z) &&
							!glm::isnan(vertices[p].Position.x) && !glm::isnan(vertices[p].Position.y) && !glm::isnan(vertices[p].Position.z));
					}
				}

				// -------- Normal attribute -----------

				else if (attribute.first == "NORMAL")
				{
					size_t normalCount = accessor.count;
					glm::vec3* normals = reinterpret_cast<glm::vec3*>(data.data());
					for (auto p = 0; p < normalCount; ++p)
					{
						vertices[p].Normal = glm::transpose(glm::inverse(glm::mat3(p_ParentTransform.GetWorldMatrix()))) * normals[p];
						vertices[p].Normal = glm::normalize(vertices[p].Normal);
						YM_CORE_ASSERT(
							!glm::isinf(vertices[p].Position.x) && !glm::isinf(vertices[p].Position.y) && !glm::isinf(vertices[p].Position.z) &&
							!glm::isnan(vertices[p].Position.x) && !glm::isnan(vertices[p].Position.y) && !glm::isnan(vertices[p].Position.z));
					}
				}

				// -------- Texcoord attribute -----------

				else if (attribute.first == "TEXCOORD_0")
				{
					size_t uvCount = accessor.count;
					glm::vec2* uvs = reinterpret_cast<glm::vec2*>(data.data());
					for (auto p = 0; p < uvCount; ++p)
					{
						vertices[p].TexCoord = uvs[p];
						if (p_FlipYTexCoord)
						{
							vertices[p].TexCoord = 1.0f - uvs[p];
						}
					}
				}

				// -------- Colour attribute -----------

				else if (attribute.first == "COLOR_0")
				{
					size_t uvCount = accessor.count;
					glm::vec4* colors = reinterpret_cast<glm::vec4*>(data.data());
					for (auto p = 0; p < uvCount; ++p)
					{
						vertices[p].Color = colors[p];
					}
				}
			}

			// -------- Indices ----------
			std::vector<uint32_t> indices;
			if (primitive.indices >= 0)
			{
				const tinygltf::Accessor& indicesAccessor = p_Model.accessors[primitive.indices];
				indices.resize(indicesAccessor.count);
				{
					// Get accessor info
					auto indexAccessor	 = p_Model.accessors.at(primitive.indices);
					auto indexBufferView = p_Model.bufferViews.at(indexAccessor.bufferView);
					auto indexBuffer	 = p_Model.buffers.at(indexBufferView.buffer);

					int componentLength = s_GLTF_COMPONENT_LENGTH_LOOKUP[indexAccessor.type];
					int componentTypeByteSize = s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP[indexAccessor.componentType];

					// Extra index data
					size_t bufferOffset = indexBufferView.byteOffset + indexAccessor.byteOffset;
					int bufferLength = static_cast<int>(indexAccessor.count) * componentLength * componentTypeByteSize;
					std::vector<uint8_t> data;
					data.resize(bufferLength);
					uint8_t* arrayData = data.data();
					memcpy(arrayData, indexBuffer.data.data() + bufferOffset, bufferLength);

					size_t indicesCount = indexAccessor.count;
					if (componentTypeByteSize == 1)
					{
						uint8_t* in = reinterpret_cast<uint8_t*>(data.data());
						for (auto iCount = 0; iCount < indicesCount; iCount++)
						{
							indices[iCount] = (uint32_t)in[iCount];
						}
					}
					else if (componentTypeByteSize == 2)
					{
						uint16_t* in = reinterpret_cast<uint16_t*>(data.data());
						for (auto iCount = 0; iCount < indicesCount; iCount++)
						{
							indices[iCount] = (uint32_t)in[iCount];
						}
					}
					else if (componentTypeByteSize == 4)
					{
						auto in = reinterpret_cast<uint32_t*>(data.data());
						for (auto iCount = 0; iCount < indicesCount; iCount++)
						{
							indices[iCount] = in[iCount];
						}
					}
					else
					{
						YM_CORE_WARN(GLTF_PREFIX "Unsupported indices data type - {}", componentTypeByteSize);
					}
				}
			}
			else
			{
				YM_CORE_WARN(GLTF_PREFIX "Missing Indices - Generating new");

				const auto& accessor = p_Model.accessors[primitive.attributes.find("POSITION")->second];
				indices.reserve(accessor.count);

				for (size_t vi = 0; vi < accessor.count; vi += 3)
				{
					indices.push_back(uint32_t(vi + 0));
					indices.push_back(uint32_t(vi + 1));
					indices.push_back(uint32_t(vi + 2));
				}
			}

			if (!hasNormals)
				Mesh::GenerateNormals(vertices.data(), uint32_t(vertices.size()), indices.data(), uint32_t(indices.size()));

			// Add mesh
			Mesh* lMesh = new Mesh(indices, vertices);

			meshes.emplace_back(lMesh);
		}

		return meshes;
	}

	static void LoadNode(Model* p_MainModel, int p_NodeIndex, const glm::mat4& p_ParentTransform, tinygltf::Model& p_Model, std::vector<Ref<Material>>& p_Materials, bool p_FlipYTexCoord)
	{
		YM_PROFILE_FUNCTION()

		if (p_NodeIndex < 0)
			return;

		auto& node = p_Model.nodes[p_NodeIndex];
		auto name  = node.name;


		Math::Transform transform;
		glm::mat4 matrix	  = glm::mat4(1.0f);
		glm::mat4 translation = glm::mat4(1.0f);
		glm::mat4 rotation	  = glm::mat4(1.0f);
		glm::mat4 scale		  = glm::mat4(1.0f);

		if (!node.scale.empty())
		{
			scale = glm::scale(scale, glm::vec3(
				static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]),
				static_cast<float>(node.scale[2]))
			);
		}

		if (!node.rotation.empty())
		{
			rotation = glm::toMat4(glm::quat(
				static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]),
				static_cast<float>(node.rotation[2]), static_cast<float>(node.rotation[3]))
			);
		}

		if (!node.translation.empty())
		{
			translation = glm::translate(translation, glm::vec3(
				static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]),
				static_cast<float>(node.translation[2]))
			);
		}

		if (!node.matrix.empty())
		{
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					matrix[x][y] = float(node.matrix.data()[y * 4 + x]);
				}
			}

			transform.SetLocalMatrix(matrix);
		}
		else
		{
			matrix = translation * rotation * scale;
			transform.SetLocalMatrix(matrix);
		}

		transform.SetWorldMatrix(p_ParentTransform);

		if (node.mesh >= 0)
		{
			int subIndex = 0;

			auto meshes = LoadMesh(p_Model, p_Model.meshes[node.mesh], p_Materials, transform, p_FlipYTexCoord);

			for (auto& mesh : meshes)
			{
				auto subname = p_Model.meshes[node.mesh].name;
				auto lMesh = Ref<Mesh>(mesh);
				lMesh->SetName(subname);

				subIndex = std::min(subIndex, (int)p_Model.meshes[node.mesh].primitives.size());
				int materialIndex = p_Model.meshes[node.mesh].primitives[subIndex].material;
				if (materialIndex >= 0)
					lMesh->SetMaterial(p_Materials[materialIndex]);

				p_MainModel->AddMesh(lMesh);

				subIndex++;
			}
		}

		if (!node.children.empty())
		{
			for (int child : node.children)
			{
				LoadNode(p_MainModel, child, transform.GetLocalMatrix(), p_Model, p_Materials, p_FlipYTexCoord);
			}
		}
	}


	void Model::LoadModelGLTF(const std::string& p_Path, bool p_FlipYTexCoord)
	{
		if (!s_MapsInitialised)
		{
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_SCALAR,					 1 });
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_VEC2,					 2 });
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_VEC3,					 3 });
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_VEC4,					 4 });
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_MAT2,					 4 });
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_MAT3,					 9 });
			s_GLTF_COMPONENT_LENGTH_LOOKUP.insert({		(int)TINYGLTF_TYPE_MAT4,					16 });


			s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP.insert({	(int)TINYGLTF_COMPONENT_TYPE_BYTE,			 1 });
			s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP.insert({	(int)TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE,	 1 });
			s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP.insert({	(int)TINYGLTF_COMPONENT_TYPE_SHORT,			 2 });
			s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP.insert({	(int)TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, 2 });
			s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP.insert({	(int)TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,	 4 });
			s_GLTF_COMPONENT_BYTE_SIZE_LOOKUP.insert({	(int)TINYGLTF_COMPONENT_TYPE_FLOAT,			 4 });

			s_MapsInitialised = true;
		}

		std::filesystem::path path = std::filesystem::path(p_Path);
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string error;
		std::string warn;

		std::string ext = path.extension().string();

		bool ret;

		if (ext == "glb") // assume binary glTF.
		{
			YM_PROFILE_SCOPE(".glb binary loading");
			ret = tinygltf::TinyGLTF().LoadBinaryFromFile(&model, &error, &warn, p_Path);
		}
		else // assume ascii glTF.
		{
			YM_PROFILE_SCOPE(".gltf loading");
			ret = tinygltf::TinyGLTF().LoadASCIIFromFile(&model, &error, &warn, p_Path);
		}

		if (!error.empty())
		{
			auto strs = SplitString(error, '\n');
			for (const auto& str : strs)
			{
				if (!str.empty())
					YM_CORE_ERROR(GLTF_PREFIX "{}", str)
			}
			return;
		}

		if (!warn.empty())
		{
			auto strs = SplitString(warn, '\n');
			for (const auto& str : strs)
			{
				if (!str.empty())
					YM_CORE_WARN(GLTF_PREFIX "{}", str)
			}
		}

		if (!ret || model.defaultScene < 0 || model.scenes.empty())
		{
			YM_CORE_ERROR(GLTF_PREFIX "Failed to parse glTF")
			return;
		}

		{
			YM_PROFILE_SCOPE("Parse GLTF Model")

			auto loadedMaterials = LoadMaterials(model);

			std::string name = path.stem().string();

			const tinygltf::Scene& gltfScene = model.scenes[std::max(0, model.defaultScene)];
			for (size_t i = 0; i < gltfScene.nodes.size(); i++)
			{
				LoadNode(this, gltfScene.nodes[i], glm::mat4(1.0f), model, loadedMaterials, p_FlipYTexCoord);
			}
		}
	}

	#pragma endregion

} // YUME
