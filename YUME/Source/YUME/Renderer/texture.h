#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/definitions.h"

// Lib
#include <glm/glm.hpp>


namespace YUME
{

	struct YM_API TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		TextureFormat Format = TextureFormat::RGBA8_SRGB;
		TextureFilter MinFilter = TextureFilter::LINEAR;
		TextureFilter MagFilter = TextureFilter::LINEAR;
		TextureWrap Wrap = TextureWrap::REPEAT;
		TextureBorderColor BorderColorFlag = TextureBorderColor::CUSTOM_FLOAT;
		// if BorderColorFlag is CUSTOM_SRGB, each component in BorderColor will be converted to int.
		// each component in BorderColor will be clamped between 0.0f and 1.0f if CUSTOM_FLOAT or between 0 and 255 if CUSTOM_SRGB
		glm::vec4 BorderColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		TextureFlags Flags = TextureFlagBits::TEXTURE_CREATE_MIPS; // no need to pass TextureFlagBits::
	};

	class YM_API Texture
	{
		public:
			virtual ~Texture() = default;

			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			virtual uint32_t GetChannels() const = 0;

			virtual uint64_t GetEstimatedSize() const = 0;

			virtual const TextureSpecification& GetSpecification() const = 0;


			virtual std::vector<unsigned char>& GetData() = 0;

			virtual void SetData(const unsigned char* p_Data, uint32_t p_Size) = 0;

			virtual void Bind(uint32_t p_Slot = 0) const = 0;
			virtual void Unbind(uint32_t p_Slot = 0) const = 0;

			virtual bool operator== (const Texture& other) const = 0;
	};


	class YM_API Texture2D : public Texture
	{
		public:
			static Ref<Texture2D> Create(const TextureSpecification& p_Spec = {});
			static Ref<Texture2D> Create(const TextureSpecification& p_Spec = {}, const unsigned char* p_Data = nullptr, uint32_t p_Size = 0);
	};
}