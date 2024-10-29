#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "uniform_buffer.h"
#include "texture.h"
#include "shader.h"


namespace YUME
{
	class YM_API RendererAPI;

	struct YM_API DescriptorSpec
	{
		uint32_t Set = 0;
		Ref<Shader> Shader;
	};

	class YM_API DescriptorSet
	{
		friend class RendererAPI;

		public:
			virtual ~DescriptorSet() = default;

			virtual void SetUniformData(const std::string& p_Name, const Ref<UniformBuffer>& p_UniformBuffer) = 0;
			virtual void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data) = 0;
			virtual void SetUniform(const std::string& p_BufferName, const std::string& p_MemberName, void* p_Data, uint32_t p_Size) = 0;

			virtual void SetTexture2D(const std::string& p_Name, const Ref<Texture2D>& p_Texture) = 0;
			virtual void SetTexture2D(const std::string& p_Name, const Ref<Texture2D>* p_TextureData, uint32_t p_Count) = 0;

			virtual void Upload() = 0;

			static Ref<DescriptorSet> Create(const DescriptorSpec& p_Spec);

		protected:
			virtual void Bind() = 0;
	};
}