#pragma once
#include "YUME/Renderer/sub_texture.h"
#include "YUME/Renderer/texture_importer.h"
#include "YUME/Core/definitions.h"

#include <variant>
#include <glm/glm.hpp>



namespace YUME
{
	struct Square {};
	struct Circle
	{
		float Thickness = 1.0f;
		float Fade = 0.005f;

		Circle() = default;
		Circle(const Circle&) = default;
	};

	using ShapeVar = std::variant<Square, Circle>;

	struct ShapeComponent
	{
		ShapeComponent() = default;
		ShapeComponent(const ShapeComponent&) = default;

		ShapeVar Shape = Square{};

		template<typename T>
		T& SetShape(const T& p_Shape = T{})
		{
			//YM_CORE_VERIFY(std::holds_alternative<T>(Shape), "T isn't a valid shape");

			Shape = p_Shape;
			return std::get<T>(Shape);
		}

		template<typename T, typename... Args>
		T& SetShape(Args&& ...p_Args)
		{
			//YM_CORE_VERIFY(std::holds_alternative<T>(Shape), "T isn't a valid shape");

			Shape = T(std::forward<Args>(p_Args)...);
			return std::get<T>(Shape);
		}

		template<typename T>
		bool IsShape()
		{
			return std::visit([](auto&& p_Arg) -> bool {
				using CurShape = std::decay_t<decltype(p_Arg)>;

				if constexpr (std::is_same_v<CurShape, T>) {
					return true;
				}
				return false;

			}, Shape);
		}

		template<typename T>
		T& Get()
		{
			YM_CORE_VERIFY(std::holds_alternative<T>(Shape), "T isn't a valid shape");
			return std::get<T>(Shape);
		}
	};

	struct SpriteComponent
	{
		SurfaceType Type = SurfaceType::Opaque; // if the color's alpha is less than 0.98f, it will be set to Transparent
		glm::vec4 Color{ 1.0f };
		std::string Path = std::string();
		Ref<SubTexture2D> Texture = nullptr;


		glm::vec2 Size = { 1.0f, 1.0f };
		glm::vec2 Scale = { 1.0f, 1.0f };
		bool BySize = true;
		glm::vec2 Offset = { 0.0f, 0.0f };

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const glm::vec4& p_Color, const std::string& p_Path = std::string(), const glm::vec2& p_Scale = {1.0f, 1.0f},
			const glm::vec2& p_Offset = {0.0f, 0.0f}, bool p_BySize = true, const glm::vec2& p_Size = {0.0f, 0.0f})
			: Color(p_Color), Path(p_Path), Offset(p_Offset), Scale(p_Scale), Size(p_Size)
		{
			if (p_Path.empty())
				return;

			SubTextureSpec spec{};
			spec.Texture = TextureImporter::LoadTexture2D(Path); // TEMPORARY, when we have the Asset Manager the texture will become an Asset Handle
			if (p_Size.x == 0.0f || p_Size.y == 0.0f)
			{
				Size.x = (float)spec.Texture->GetWidth();
				Size.y = (float)spec.Texture->GetHeight();
			}
			spec.Size = Size;
			spec.BySize = BySize;
			spec.Offset = Offset;
			spec.Scale = Scale;

			Texture = SubTexture2D::Create(spec);
		}
		// TEMPORARY, when we have the Asset Manager the texture will become an Asset Handle
		// Then this constructor will be deleted
		SpriteComponent(const glm::vec4& p_Color, const Ref<Texture2D>& p_Texture, const glm::vec2& p_Scale = { 1.0f, 1.0f },
			const glm::vec2& p_Offset = { 0.0f, 0.0f }, bool p_BySize = true, const glm::vec2& p_Size = { 0.0f, 0.0f })
			: Color(p_Color), Offset(p_Offset), Scale(p_Scale), Size(p_Size)
		{
			if (p_Texture == nullptr)
				return;

			SubTextureSpec spec{};
			spec.Texture = p_Texture;
			if (p_Size.x == 0.0f || p_Size.y == 0.0f)
			{
				Size.x = (float)spec.Texture->GetWidth();
				Size.y = (float)spec.Texture->GetHeight();
			}
			spec.Size = Size;
			spec.BySize = BySize;
			spec.Offset = Offset;
			spec.Scale = Scale;

			Texture = SubTexture2D::Create(spec);
		}
	};

} // YUME