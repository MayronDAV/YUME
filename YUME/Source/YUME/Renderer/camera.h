#pragma once
#include "YUME/Core/base.h"
#include <glm/glm.hpp>


namespace YUME
{
	class YM_API Camera
	{
		public:
			Camera() = default;
			explicit Camera(const glm::mat4& p_ViewProjection, const glm::vec3& p_Position = { 0.0f, 0.0f, 0.0f })
				: m_ViewProjection(p_ViewProjection), m_Position(p_Position) {}
			virtual ~Camera() = default;

			void SetPosition(const glm::vec3& p_Position) { m_Position = p_Position; }
			void SetViewProjection(const glm::mat4& p_ViewProjection) { m_ViewProjection = p_ViewProjection; }
			const glm::mat4& GetViewProjection() const { return m_ViewProjection; }

			glm::vec3& GetPosition() { return m_Position; }
			const glm::vec3& GetPosition() const { return m_Position; }

		protected:
			glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
			glm::mat4 m_ViewProjection{ 1.0f };

	};
}