#pragma once

#include "YUME/Core/uuid.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>



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
		glm::vec3 Translation{ 0.0f };
		glm::vec3 Rotation{ 0.0f };
		glm::vec3 Scale{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& p_Translation, const glm::vec3& p_Scale = { 1.0f, 1.0f, 1.0f }, const glm::vec3& p_Rotation = { 0.0f, 0.0f, 0.0f })
			: Translation(p_Translation), Scale(p_Scale), Rotation(p_Rotation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) *
				rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

} // YUME