#include "YUME/yumepch.h"
#include "mesh.h"
#include "renderer_command.h"



namespace YUME
{

	Mesh::Mesh(const std::vector<uint32_t>& p_Indices, const std::vector<MeshVertex>& p_Vertices)
	{
		YM_PROFILE_FUNCTION()

		m_VertexBuffer = VertexBuffer::Create(p_Vertices.data(), p_Vertices.size() * sizeof(MeshVertex));
		m_IndexBuffer = IndexBuffer::Create(p_Indices.data(), (uint32_t)p_Indices.size());
	}

	void Mesh::BindMaterial(CommandBuffer* p_CommandBuffer, const Ref<Shader>& p_Shader, bool p_PBR)
	{
		m_Material->Bind(p_CommandBuffer, p_Shader, p_PBR);
	}

	void Mesh::GenerateNormals(MeshVertex* p_Vertices, uint32_t p_VertexCount, uint32_t* p_Indices, uint32_t p_IndexCount)
	{
		YM_PROFILE_FUNCTION()

		glm::vec3* normals = new glm::vec3[p_VertexCount];

		for (uint32_t i = 0; i < p_VertexCount; ++i)
		{
			normals[i] = glm::vec3();
		}

		if (p_Indices)
		{
			for (uint32_t i = 0; i < p_IndexCount; i += 3)
			{
				const int a = p_Indices[i + 0];
				const int b = p_Indices[i + 1];
				const int c = p_Indices[i + 2];

				const glm::vec3 _normal = glm::cross((p_Vertices[b].Position - p_Vertices[a].Position), (p_Vertices[c].Position - p_Vertices[a].Position));

				normals[a] += _normal;
				normals[b] += _normal;
				normals[c] += _normal;
			}
		}
		else
		{
			for (uint32_t i = 0; i < p_VertexCount - 3; i += 3)
			{
				glm::vec3& a = p_Vertices[i + 0].Position;
				glm::vec3& b = p_Vertices[i + 1].Position;
				glm::vec3& c = p_Vertices[i + 2].Position;

				const glm::vec3 _normal = glm::cross(b - a, c - a);

				normals[i + 0] = _normal;
				normals[i + 1] = _normal;
				normals[i + 2] = _normal;
			}
		}

		for (uint32_t i = 0; i < p_VertexCount; ++i)
		{
			p_Vertices[i].Normal = glm::normalize(normals[i]);
		}

		delete[] normals;
	}


} // YUME