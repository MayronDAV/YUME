#include "YUME/yumepch.h"
#include "Platform/Vulkan/Core/vulkan_window.h"

#include "YUME/Events/application_event.h"
#include "YUME/Events/mouse_event.h"
#include "YUME/Events/key_event.h"
#include "YUME/Core/engine.h"

// Lib
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace YUME
{
	namespace Utils
	{
		int CursorModeToGLFWCursorMode(CursorMode p_Mode)
		{
			switch (p_Mode)
			{
				case YUME::CursorMode::HIDDEN:   return GLFW_CURSOR_HIDDEN;
				case YUME::CursorMode::DISABLED: return GLFW_CURSOR_DISABLED;
				case YUME::CursorMode::NORMAL:   return GLFW_CURSOR_NORMAL;
			}

			YM_CORE_ERROR("Unknown cursor mode!")
			return 0;
		}
	}

	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int p_Error, const char* p_Description)
	{
		YM_CORE_ERROR("GLFW Error ({0}): {1}", p_Error, p_Description)
	}

	VulkanWindow::VulkanWindow(const WindowProps& p_Props)
	{
		m_Data.Context = CreateUnique<VulkanContext>();
		Init(p_Props);
	}

	VulkanWindow::~VulkanWindow()
	{
		Shutdown();
	}

	void VulkanWindow::OnUpdate()
	{
		YM_PROFILE_FUNCTION()

		glfwPollEvents();
	}

	void VulkanWindow::BeginFrame()
	{
		m_Data.Context->Begin();
	}

	void VulkanWindow::EndFrame()
	{
		m_Data.Context->End();
	}

	GraphicsContext* VulkanWindow::GetContext()
	{
		return m_Data.Context.get();
	}

	void VulkanWindow::SetCursorMode(CursorMode p_Mode)
	{
		int mode = Utils::CursorModeToGLFWCursorMode(p_Mode);
		if (mode != 0)
			glfwSetInputMode(m_Window, GLFW_CURSOR, mode);
	}

	void VulkanWindow::Init(const WindowProps& p_Props)
	{
		YM_PROFILE_FUNCTION()

		m_Data.Title = p_Props.Title;
		m_Data.Width = p_Props.Width;
		m_Data.Height = p_Props.Height;

		YM_CORE_INFO("Creating window {0} ({1}, {2})", p_Props.Title, p_Props.Width, p_Props.Height);

		if (s_GLFWWindowCount == 0)
		{
			int success = glfwInit();
			YM_CORE_ASSERT(success, "Could not initialize GLFW!")
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		{
			YM_PROFILE_SCOPE("VulkanWindow::Init - glfwCreateWindow")
			m_Window = glfwCreateWindow((int)p_Props.Width, (int)p_Props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		}
		YM_CORE_ASSERT(m_Window)
		m_Data.Context->Init(m_Data.Title.c_str(), m_Window);
		++s_GLFWWindowCount;

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		YM_CORE_ASSERT(monitor)

		int count = 0;
		const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

		for (int i = 0; i < count; i++) {
			m_Resolutions.emplace_back(modes[i].width, modes[i].height, modes[i].refreshRate);
		}

		glfwSetWindowUserPointer(m_Window, &m_Data);
		//SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* p_Window, int p_Width, int p_Height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
			data.Width = p_Width;
			data.Height = p_Height;
			data.Context->OnResize(data.Width, data.Height);

			WindowResizeEvent event(p_Width, p_Height);
			data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* p_Window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* p_Window, int p_Key, int p_Scancode, int p_Action, int p_Mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			switch (p_Action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(p_Key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(p_Key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(p_Key, true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* p_Window, unsigned int p_Keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			KeyTypedEvent event(p_Keycode);
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* p_Window, int p_Button, int p_Action, int p_Mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			switch (p_Action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(p_Button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(p_Button);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* p_Window, double p_Xoffset, double p_Yoffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			MouseScrolledEvent event((float)p_Xoffset, (float)p_Yoffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* p_Window, double p_Xpos, double p_Ypos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

			MouseMovedEvent event((float)p_Xpos, (float)p_Ypos);
			data.EventCallback(event);
		});
	}

	void VulkanWindow::Shutdown()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_WARN("{} window shutdown", m_Data.Title)
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

	void VulkanWindow::SetVSync(bool p_Enabled)
	{
		YM_CORE_TRACE("Set vsync {}", p_Enabled)
		m_Data.Vsync = p_Enabled;
	}

	bool VulkanWindow::IsVSync() const
	{
		return m_Data.Vsync;
	}
}
