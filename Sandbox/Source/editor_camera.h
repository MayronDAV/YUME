#pragma once

#include "YUME/yume.h"


namespace YUME
{
	class EditorCamera : public Camera
	{
		public:
			EditorCamera() = default;
			EditorCamera(float p_Fov, float p_AspectRatio, float p_NearClip, float p_FarClip, bool p_FlipY = true);

			void OnUpdate(const Timestep& p_Ts);
			void OnEvent(Event& p_Event);

			inline float GetZoom() const { return m_Zoom; }
			inline void SetZoom(float p_Zoom) { m_Zoom = std::min(std::max(p_Zoom, 0.1f), 1.0f); }

			inline void SetViewportSize(float p_Width, float p_Height)
			{
				if ((p_Width > 0 && p_Height > 0) && (m_ViewportWidth != p_Width && m_ViewportHeight != p_Height))
					m_ViewportWidth = p_Width; m_ViewportHeight = p_Height; UpdateProjection();
			}

			float GetPitch() const { return m_Pitch; }
			float GetYaw() const { return m_Yaw; }

			void SetFocus(bool p_Focus) { m_IsFocused = p_Focus; }

		private:
			void UpdateProjection();
			void UpdateView();
			void UpdateCameraVectors();

			glm::vec3 CalculatePosition() const;

			void ProcessKeyboard(const Timestep& p_Ts);

			void ProcessMouseMovement(const glm::vec2& p_delta, bool p_constrainPitch = true);

			bool OnMouseScroll(MouseScrolledEvent& p_Event);
			void MouseZoom(float p_Delta);
			float ZoomSpeed() const;

		private:
			bool m_IsFocused = true;
			bool m_FlipY = true;

			glm::vec3 m_Front = { 0.0f, 0.0f, -1.0f };
			glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };
			glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_WorldUp = { 0.0f, 1.0f, 0.0f };
			float m_MovementSpeed = 5.0f;
			float m_MouseSensitivity = 0.1f;

			float m_FOV = 45.0f;
			float m_AspectRatio = 1.778f;
			float m_NearClip = 0.1f;
			float m_FarClip = 1000.0f;
			float m_Pitch = 0.0f;
			float m_Yaw = 0.0f;

			glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
			glm::mat4 m_ViewMatrix{ 1.0f };
			glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

			float m_ViewportWidth = 1280;
			float m_ViewportHeight = 720;

			float m_Zoom = 1.0f;
			float m_NewZoom = m_Zoom;
	};
}