#pragma once

#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "vertex_array.h"
#include "pipeline.h"
#include "uniform_buffer.h"
#include "YUME/Core/definitions.h"

// std
#include <string_view>

// Lib
#include <glm/glm.hpp>
#include "texture.h"



namespace YUME
{

	class YM_API Shader
	{
		public:
			virtual ~Shader() = default;

			virtual void Bind() = 0;
			virtual void Unbind() = 0;
		
			virtual const std::string_view& GetName() const = 0;

			virtual void PushFloat(const std::string& p_Name, float p_Value) = 0;
			virtual void PushFloat3(const std::string& p_Name, const glm::vec3& p_Value) = 0;
			virtual void PushFloat4(const std::string& p_Name, const glm::vec4& p_Value) = 0;
			virtual void PushMat4(const std::string& p_Name, const glm::mat4& p_Value) = 0;
			virtual void PushInt(const std::string& p_Name, int p_Value) = 0;

			virtual void AddVertexArray(const Ref<VertexArray>& p_VertexArray) = 0;

			static Ref<Shader> Create(const std::string& p_ShaderPath);
	};
}