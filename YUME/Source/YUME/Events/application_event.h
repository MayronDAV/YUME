#pragma once

#include "YUME/Events/event.h"



namespace YUME
{

	class YM_PUBLIC WindowResizeEvent : public Event
	{
		public:
			WindowResizeEvent(uint32_t p_Width, uint32_t p_Height)
				: m_Width(p_Width), m_Height(p_Height) {}

			inline uint32_t GetWidth() const { return m_Width; }
			inline uint32_t GetHeight() const { return m_Height; }

			std::string ToString() const override
			{
				std::stringstream ss;
				ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
				return ss.str();
			}

			EVENT_CLASS_TYPE(WindowResize)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

		private:
			uint32_t m_Width;
			uint32_t m_Height;
	};

	class YM_PUBLIC WindowCloseEvent : public Event
	{
		public:
			WindowCloseEvent() = default;

			EVENT_CLASS_TYPE(WindowClose)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class YM_PUBLIC AppTickEvent : public Event
	{
		public:
			AppTickEvent() = default;

			EVENT_CLASS_TYPE(AppTick)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class YM_PUBLIC AppUpdateEvent : public Event
	{
		public:
			AppUpdateEvent() = default;

			EVENT_CLASS_TYPE(AppUpdate)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class YM_PUBLIC AppRenderEvent : public Event
	{
		public:
			AppRenderEvent() = default;

			EVENT_CLASS_TYPE(AppRender)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

	class YM_PUBLIC WindowDropEvent : public Event
	{
		public:
			explicit WindowDropEvent(const std::vector<std::filesystem::path>& p_Paths)
				: m_Paths(p_Paths) {}

			explicit WindowDropEvent(std::vector<std::filesystem::path>&& p_Paths)
				: m_Paths(std::move(p_Paths)) {}

			const std::vector<std::filesystem::path>& GetPaths() const { return m_Paths; }

			EVENT_CLASS_TYPE(WindowDrop)
			EVENT_CLASS_CATEGORY(EventCategoryApplication)

		private:
			std::vector<std::filesystem::path> m_Paths;
	};
}
