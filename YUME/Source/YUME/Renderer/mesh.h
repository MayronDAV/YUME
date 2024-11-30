#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/definitions.h"
#include "YUME/Core/reference.h"
#include "buffer.h"
#include "material.h"

#include <glm/glm.hpp>
#include "descriptor_set.h"




namespace YUME
{
	class YM_API Model;

	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec4 Color;
	};

	class YM_API Mesh
	{
		friend class Model;

		public:
			Mesh() = default;
			Mesh(const std::vector<uint32_t>& p_Indices, const std::vector<MeshVertex>& p_Vertices);

			const Ref<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
			const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
		
			void BindMaterial(CommandBuffer* p_CommandBuffer, const Ref<Shader>& p_Shader, bool p_PBR = true);

			void SetName(const std::string& p_Name) { m_Name = p_Name; }

			void SetMaterial(const Ref<Material>& p_Material) { m_Material = p_Material; }
			const Ref<Material>& GetMaterial() { return m_Material; }

			static void GenerateNormals(MeshVertex* p_Vertices, uint32_t p_VertexCount, uint32_t* p_Indices, uint32_t p_IndexCount);

		private:
			Ref<Material> m_Material;
			Ref<VertexBuffer> m_VertexBuffer;
			Ref<IndexBuffer> m_IndexBuffer;

			std::string m_Name = "Mesh";
	};


} // YUME