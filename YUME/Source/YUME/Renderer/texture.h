#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/reference.h"
#include "YUME/Core/definitions.h"

// Lib
#include <glm/glm.hpp>


namespace YUME
{

	struct YM_API TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		TextureUsage Usage = TextureUsage::TEXTURE_SAMPLED;
		TextureFormat Format = TextureFormat::RGBA8_SRGB;
		TextureFilter MinFilter = TextureFilter::LINEAR;
		TextureFilter MagFilter = TextureFilter::LINEAR;
		TextureWrap WrapU = TextureWrap::REPEAT;
		TextureWrap WrapV = TextureWrap::REPEAT;
		TextureWrap WrapW = TextureWrap::REPEAT;
		TextureBorderColor BorderColorFlag = TextureBorderColor::CUSTOM_FLOAT;
		// if BorderColorFlag is CUSTOM_SRGB, each component in BorderColor will be converted to int.
		// each component in BorderColor will be clamped between 0.0f and 1.0f if CUSTOM_FLOAT or between 0 and 255 if CUSTOM_SRGB
		glm::vec4 BorderColor = { 0.0f, 0.0f, 0.0f, 1.0f };
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

			static bool IsDepthStencilFormat(TextureFormat p_Format)
			{
				return p_Format == TextureFormat::D24_UNORM_S8_UINT || p_Format == TextureFormat::D16_UNORM_S8_UINT ||
					p_Format == TextureFormat::D32_FLOAT_S8_UINT;
			}

			static bool IsDepthFormat(TextureFormat p_Format)
			{
				return p_Format == TextureFormat::D16_UNORM || p_Format == TextureFormat::D32_FLOAT ||
					p_Format == TextureFormat::D24_UNORM_S8_UINT || p_Format == TextureFormat::D16_UNORM_S8_UINT ||
					p_Format == TextureFormat::D32_FLOAT_S8_UINT;
			}

			static bool IsStencilFormat(TextureFormat p_Format)
			{
				return p_Format == TextureFormat::D24_UNORM_S8_UINT || p_Format == TextureFormat::D16_UNORM_S8_UINT ||
					p_Format == TextureFormat::D32_FLOAT_S8_UINT;
			}

			bool IsSampled() const { return GetSpecification().Usage == TextureUsage::TEXTURE_SAMPLED; }
			bool IsColorAttachment() const { return GetSpecification().Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT; }
			bool IsDepthStencilAttachment() const { return GetSpecification().Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT; }

			virtual bool operator== (const Texture& p_Other) const = 0;
	};


	class YM_API Texture2D : public Texture
	{
		public:
			virtual void Resize(uint32_t p_Width, uint32_t p_Height) = 0;

			static Ref<Texture2D> Create(const TextureSpecification& p_Spec = {});
			static Ref<Texture2D> Create(const TextureSpecification& p_Spec, const unsigned char* p_Data, uint32_t p_Size);

			static Ref<Texture2D> Get(const TextureSpecification& p_Spec = {});
			static void ClearCache();
			static void DeleteUnusedCache();
	};
}