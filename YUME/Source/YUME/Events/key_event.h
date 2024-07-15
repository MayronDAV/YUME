#pragma once

#include "YUME/Events/event.h"



namespace YUME 
{

	class YM_API KeyEvent : public Event
	{
		public:
			inline int GetKeyCode() const { return m_KeyCode; }

			EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

		protected:
			explicit KeyEvent(int p_Keycode)
				: m_KeyCode(p_Keycode) {}

			int m_KeyCode;
	};

	class YM_API KeyPressedEvent : public KeyEvent
	{
		public:
			KeyPressedEvent(int p_Keycode, bool p_IsRepeat = false)
				: KeyEvent(p_Keycode), m_IsRepeat(p_IsRepeat) {}

			bool IsRepeat() const { return m_IsRepeat; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "KeyPressedEvent: " << m_KeyCode << " (repeat = " << m_IsRepeat << ")";
				return ss.str();
			}

			EVENT_CLASS_TYPE(KeyPressed)

		private:
			bool m_IsRepeat;
	};

	class YM_API KeyReleasedEvent : public KeyEvent
	{
		public:
			explicit KeyReleasedEvent(int p_Keycode)
				: KeyEvent(p_Keycode) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "KeyReleasedEvent: " << m_KeyCode;
				return ss.str();
			}

			EVENT_CLASS_TYPE(KeyReleased)
	};

	class YM_API KeyTypedEvent : public KeyEvent
	{
		public:
			explicit KeyTypedEvent(int p_Keycode)
				: KeyEvent(p_Keycode) {}

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "KeyTypedEvent: " << m_KeyCode;
				return ss.str();
			}

			EVENT_CLASS_TYPE(KeyTyped)
	};
}
