#pragma once
#include "YUME/Core/base.h"
#include <glm/glm.hpp>


namespace YUME
{
	class YM_API Camera
	{
		public:
			Camera() = default;
			explicit Camera(const glm::mat4& p_Projection)
				: m_Projection(p_Projection) {}
			virtual ~Camera() = default;

			const glm::mat4& GetProjection() const { return m_Projection; }

		protected:
			glm::mat4 m_Projection{ 1.0f };

	};
}