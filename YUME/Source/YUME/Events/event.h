#pragma once

#include "YUME/Core/base.h"

#include <string>
#include <sstream>
#include <functional>



namespace YUME
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowDrop,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = YM_BIT(0),
		EventCategoryInput = YM_BIT(1),
		EventCategoryKeyboard = YM_BIT(2),
		EventCategoryMouse = YM_BIT(3),
		EventCategoryMouseButton = YM_BIT(4)
	};

#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::##type; }\
								virtual EventType GetEventType() const override { return GetStaticType(); }\
								virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class YM_PUBLIC Event
	{
		public:
			virtual ~Event() = default;

			bool Handled = false;

			virtual EventType GetEventType() const = 0;
			virtual const char* GetName() const = 0;
			virtual int GetCategoryFlags() const = 0;
			virtual std::string ToString() const { return GetName(); }

			inline bool IsInCategory(EventCategory p_Category) const
			{
				return GetCategoryFlags() & p_Category;
			}
	};

	class YM_PUBLIC EventDispatcher
	{
		private:
			template<typename T>
			using EventFn = std::function<bool(T&)>;

		public:
			explicit EventDispatcher(Event& p_Event)
				: m_Event(p_Event)
			{
			}

			template<typename T>
			bool Dispatch(EventFn<T> p_Func)
			{
				if (m_Event.GetEventType() == T::GetStaticType())
				{
					m_Event.Handled = p_Func(*(T*)&m_Event);
					return true;
				}
				return false;
			}

		private:
			Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& event) {
		return os << event.ToString();
	}
}

