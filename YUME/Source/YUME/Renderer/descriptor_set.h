#pragma once
#include "YUME/Core/base.h"
#include "uniform_buffer.h"
#include "texture.h"
#include "shader.h"


namespace YUME
{

	class YM_API DescriptorSet
	{
		public:
			virtual ~DescriptorSet() = default;

			virtual void Bind(uint32_t p_Set = 0) = 0;
			virtual void Unbind() = 0;

			virtual void UploadUniform(uint32_t p_Binding, const Ref<UniformBuffer>& p_UniformBuffer) = 0;
			virtual void UploadTexture2D(uint32_t p_Binding, const Ref<Texture2D>& p_Texture) = 0;
			virtual void UploadTexture2D(uint32_t p_Binding, const Ref<Texture2D>* p_TextureData, uint32_t p_Count) = 0;

			static Ref<DescriptorSet> Create(const Ref<Shader>& p_Shader);
	};
}