#pragma once

#include "YUME/Events/event.h"



namespace YUME
{

	class YM_PUBLIC MouseMovedEvent : public Event
	{
		public:
			MouseMovedEvent(float p_X, float p_Y)
				: m_MouseX(p_X), m_MouseY(p_Y) {}

			inline float GetX() const { return m_MouseX; }
			inline float GetY() const { return m_MouseY; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
				return ss.str();
			}

			EVENT_CLASS_TYPE(MouseMoved)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

		private:
			float m_MouseX;
			float m_MouseY;
	};

	class YM_PUBLIC MouseScrolledEvent : public Event
	{
		public:
			MouseScrolledEvent(float p_Xoffset, float p_Yoffset)
				: m_XOffset(p_Xoffset), m_YOffset(p_Yoffset) {}

			inline float GetXOffset() const { return m_XOffset; }
			inline float GetYOffset() const { return m_YOffset; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
				return ss.str();
			}

			EVENT_CLASS_TYPE(MouseScrolled)
			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

		private:
			float m_XOffset;
			float m_YOffset;
	};

	class YM_PUBLIC MouseButtonEvent : public Event
	{
		public:
			inline int GetMouseButton() const { return m_Button; }

			EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

		protected:
			explicit MouseButtonEvent(int p_Button)
				: m_Button(p_Button) {}

			int m_Button;
	};

	class YM_PUBLIC MouseButtonPressedEvent : public MouseButtonEvent
	{
		public:
			explicit MouseButtonPressedEvent(int p_Button)
				: MouseButtonEvent(p_Button) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "MouseButtonPressedEvent: " << m_Button;
				return ss.str();
			}

			EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class YM_PUBLIC MouseButtonReleasedEvent : public MouseButtonEvent
	{
		public:
			explicit MouseButtonReleasedEvent(int p_Button)
				: MouseButtonEvent(p_Button) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "MouseButtonReleasedEvent: " << m_Button;
				return ss.str();
			}

			EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}