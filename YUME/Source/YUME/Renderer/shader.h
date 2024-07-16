#pragma once

#include "YUME/Core/base.h"

// std
#include <string_view>


namespace YUME
{
	class YM_API Shader
	{
		public:
			virtual ~Shader() = default;

			virtual void Bind() = 0;
			virtual void Unbind() = 0;
		
			virtual const std::string_view& GetName() const = 0;

			virtual void UploadFloat(const std::string_view& p_Name, float p_Value) = 0;
			virtual void UploadInt(const std::string_view& p_Name, int p_Value) = 0;

			static Ref<Shader> Create(const std::string& p_ShaderPath);
	};
}