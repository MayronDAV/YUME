#pragma once
#include "YUME/Core/base.h"

// lib
#include <glm/glm.hpp>



namespace YUME::Math
{
	struct YM_API WorldMatrix
	{
		WorldMatrix() = default;
		WorldMatrix(const glm::mat4& p_Matrix);

		glm::mat4 Matrix{ 1.0f };

		glm::vec3 Translation{ 0.0f };
		glm::vec3 Rotation{ 0.0f };
		glm::vec3 Scale{ 1.0f };
	};

	class YM_API Transform
	{
		public:
			Transform();
			~Transform() = default;

			void SetWorldMatrix(const glm::mat4& p_Matrix);
			void SetLocalMatrix(const glm::mat4& p_Matrix);

			void SetLocalTranslation(const glm::vec3& p_Vector);
			void SetLocalRotation(const glm::vec3& p_Vector);
			void SetLocalScale(const glm::vec3& p_Vector);

			const glm::vec3& GetLocalTranslation() const;
			const glm::vec3& GetLocalRotation() const;
			const glm::vec3& GetLocalScale() const;

			const glm::vec3& GetWorldTranslation() const;
			const glm::vec3& GetWorldRotation() const;
			const glm::vec3& GetWorldScale() const;

			const glm::mat4& GetWorldMatrix() const;
			glm::mat4 GetLocalMatrix() const;

			static bool Decompose(const glm::mat4& p_Matrix, glm::vec3& p_Translation, glm::vec3& p_Scale, glm::vec3& p_Rotation);

		private:
			WorldMatrix m_WorldMatrix;

			glm::vec3 m_LocalTranslation;
			glm::vec3 m_LocalRotation;
			glm::vec3 m_LocalScale;
	};


} // YUME::Math