#pragma once
#include "YUME/Core/base.h"
#include <glm/glm.hpp>


namespace YUME
{
	class YM_API Camera
	{
		public:
			Camera() = default;
			explicit Camera(const glm::mat4& p_Projection, const glm::mat4& p_View, const glm::vec3& p_Position = { 0.0f, 0.0f, 0.0f })
				: m_Projection(p_Projection), m_View(p_View), m_Position(p_Position) {}
			virtual ~Camera() = default;

			void SetPosition(const glm::vec3& p_Position) { m_Position = p_Position; }
			void SetProjection(const glm::mat4& p_Projection) { m_Projection = p_Projection; }
			void SetView(const glm::mat4& p_View) { m_View = p_View; }

			const glm::mat4& GetProjection() const { return m_Projection; }
			const glm::mat4& GetView() const { return m_View; }
			glm::vec3& GetPosition() { return m_Position; }
			const glm::vec3& GetPosition() const { return m_Position; }

		protected:
			glm::vec3 m_Position	= { 0.0f, 0.0f, 0.0f };
			glm::mat4 m_Projection  = { 1.0f };
			glm::mat4 m_View		= { 1.0f };

	};
}