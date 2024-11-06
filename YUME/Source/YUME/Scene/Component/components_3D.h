#pragma once

#include "YUME/Core/reference.h"
#include "YUME/Renderer/model.h"

#include <glm/glm.hpp>



namespace YUME
{
	struct ModelComponent
	{
		Ref<Model> ModelRef = nullptr;

		void LoadModel(const std::string& p_Path, bool p_FlipYTexCoord = true)
		{
			if (ModelRef)
			{
				ModelRef->LoadModel(p_Path, p_FlipYTexCoord);
				return;
			}

			ModelRef = CreateRef<Model>(p_Path, p_FlipYTexCoord);
		}

		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;
		ModelComponent(const std::string& p_Path, bool p_FlipYTexCoord = true)
		{
			ModelRef = CreateRef<Model>(p_Path, p_FlipYTexCoord);
		}
		ModelComponent(const Ref<Model>& p_Model)
			: ModelRef(p_Model) {}
	};

	enum class LightType
	{
		Point = 0,
		Directional
	};

	struct LightComponent
	{
		LightType Type = LightType::Directional;
		
		glm::vec3 Color{ 1.0f };
		glm::vec3 Direction{ glm::normalize(glm::vec3{1.0f, -1.0f, 0.0f}) };

		LightComponent() = default;
		LightComponent(const LightComponent&) = default;
		LightComponent(LightType p_Type, const glm::vec3& p_Color = { 1.0f, 1.0f, 1.0f }, const glm::vec3& p_Direction = { glm::normalize(glm::vec3{1.0f, -1.0f, 0.0f}) })
		: Type(p_Type), Color(p_Color), Direction(p_Direction) {}
	};

} // YUME