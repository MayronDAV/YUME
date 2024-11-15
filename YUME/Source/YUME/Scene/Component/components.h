#pragma once

#include "YUME/Core/uuid.h"
#include "YUME/Math/transform.h"



namespace YUME
{
	struct YM_API IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(const UUID& p_ID)
			:ID(p_ID) {}
	};

	struct YM_API TagComponent
	{
		std::string Tag = "";

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& p_Tag)
			:Tag(p_Tag) {}
	};

	struct YM_API TransformComponent
	{
		Math::Transform Transform;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::Transform& p_Transform) : Transform(p_Transform) {}
		TransformComponent(const glm::vec3& p_Translate, const glm::vec3& p_Scale = { 1.0f, 1.0f, 1.0f }, const glm::vec3& p_Rotation = { 0.0f, 0.0f, 0.0f })
		{
			Transform.SetLocalTranslation(p_Translate);
			Transform.SetLocalScale(p_Scale);
			Transform.SetLocalRotation(p_Rotation);
		}
	};

} // YUME