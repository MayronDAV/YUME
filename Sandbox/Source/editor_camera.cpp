#include "editor_camera.h"



namespace YUME
{
	EditorCamera::EditorCamera(float p_Fov, float p_AspectRatio, float p_NearClip, float p_FarClip, bool p_FlipY)
		: Camera(glm::perspective(glm::radians(p_Fov), p_AspectRatio, p_NearClip, p_FarClip)), m_FOV(p_Fov),
		m_AspectRatio(p_AspectRatio), m_NearClip(p_NearClip), m_FarClip(p_FarClip), m_FlipY(p_FlipY)
	{
		m_Position = CalculatePosition();
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_ViewProjection = glm::perspective(glm::radians(m_FOV * m_Zoom), m_AspectRatio, m_NearClip, m_FarClip);
		if (m_FlipY)
		{
			m_ViewProjection[1][1] *= -1;
		}
		m_ViewProjection *= m_ViewMatrix;
	}

	void EditorCamera::UpdateView()
	{
		m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);

		if (m_NewZoom != m_Zoom)
		{
			m_Zoom = m_NewZoom;
			UpdateProjection();
		}
	}

	void EditorCamera::UpdateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

		m_Front = glm::normalize(front);
		m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));
	}

	float EditorCamera::ZoomSpeed() const
	{
		float zoom = m_Zoom * 0.35f;
		zoom = std::max(zoom, 0.0f);
		float speed = zoom * zoom;
		speed = std::min(speed, 1.0f); // max speed = 1

		return speed;
	}

	void EditorCamera::OnUpdate(const Timestep& p_Ts)
	{
		if (!m_IsFocused)
			return;

		glm::vec2 mouse{ Input::GetMouseX(), Input::GetMouseY() };
		glm::vec2 delta = (mouse - m_InitialMousePosition);
		m_InitialMousePosition = mouse;


		if (!Input::IsKeyPressed(Key::C) && m_Zoom != 1.0f)
		{
			m_Zoom = 1.0f;
			m_NewZoom = m_Zoom;
		}


		if (Input::IsMouseButtonPressed(Mouse::Button_2))
		{
			Application::Get().GetWindow().SetCursorMode(CursorMode::DISABLED);

			auto newDelta = delta * 100.0f * (float)p_Ts;
			ProcessMouseMovement(newDelta);

			ProcessKeyboard(p_Ts);
		}
		else
		{
			Application::Get().GetWindow().SetCursorMode(CursorMode::NORMAL);
		}

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& p_Event)
	{
		EventDispatcher dispatcher(p_Event);
		dispatcher.Dispatch<MouseScrolledEvent>(YM_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		if (Input::IsKeyPressed(Key::C))
		{
			float delta = e.GetYOffset() * 0.1f;
			MouseZoom(delta);
			UpdateView();
		}

		return true;
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_NewZoom -= delta * ZoomSpeed();
		if (m_NewZoom < 0.1f)
		{
			m_FocalPoint += m_Front;
			m_NewZoom = 0.1f;
		}
		//m_NewZoom = std::max(m_NewZoom, 0.1f);
		m_NewZoom = std::min(m_NewZoom, 1.0f);
	}

	glm::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - m_Front;
	}

	void EditorCamera::ProcessKeyboard(const Timestep& p_Ts)
	{
		float speed = m_MovementSpeed * (float)p_Ts;

		if (Input::IsKeyPressed(Key::LeftShift))
			speed = speed * 2.5f;

		glm::vec3 frontXZ = glm::normalize(glm::vec3(m_Front.x, 0.0f, m_Front.z));

		if (Input::IsKeyPressed(Key::W))
			m_Position += frontXZ * speed;
		if (Input::IsKeyPressed(Key::S))
			m_Position -= frontXZ * speed;

		if (Input::IsKeyPressed(Key::A))
			m_Position -= m_Right * speed;
		if (Input::IsKeyPressed(Key::D))
			m_Position += m_Right * speed;


		if (Input::IsKeyPressed(Key::Space))
			m_Position += m_Up * speed;
		if (Input::IsKeyPressed(Key::LeftControl))
			m_Position -= m_Up * speed;
	}

	void EditorCamera::ProcessMouseMovement(const glm::vec2& p_Delta, bool p_ConstrainPitch)
	{
		float yawSign = m_Up.y < 0.0f ? -1.0f : 1.0f;
		m_Yaw += yawSign * p_Delta.x * m_MouseSensitivity;
		float pitchSign = m_Up.y < 0.0f ? 1.0f : -1.0f;
		m_Pitch += pitchSign * p_Delta.y * m_MouseSensitivity;

		if (p_ConstrainPitch)
		{
			if (m_Pitch > 89.0f)
				m_Pitch = 89.0f;
			if (m_Pitch < -89.0f)
				m_Pitch = -89.0f;
		}

		UpdateCameraVectors();
	}
}
