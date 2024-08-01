#include "YUME/yumepch.h"
#include "vulkan_imgui_layer.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"
#include "YUME/Core/application.h"
#include "Platform/Vulkan/Core/vulkan_window.h"

// Lib
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>



static ImGui_ImplVulkanH_Window g_WindowData;
static VkAllocationCallbacks* g_Allocator = nullptr;
static VkDescriptorPool g_DescriptorPool = nullptr;

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}


namespace YUME
{
	void DrawImgui(VkCommandBuffer p_Cmd, const Ref<VulkanRenderPass>& p_RenderPass)
	{
		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto extent = VulkanSwapchain::Get().GetExtent2D();

		p_RenderPass->Begin(p_Cmd, context->GetSwapchainFramebuffer().Get(), extent.width, extent.height);
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), p_Cmd);
		}
		p_RenderPass->End(p_Cmd);
	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{
		m_RenderPass.reset();

		YM_CORE_TRACE("Destroying vulkan imgui descriptor pool...")
		vkDestroyDescriptorPool(VulkanDevice::Get().GetDevice(), g_DescriptorPool, VK_NULL_HANDLE);
	}

	void VulkanImGuiLayer::Init()
	{
		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto& device = VulkanDevice::Get().GetDevice();
		auto glfwWindow = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();

		ImGui_ImplVulkanH_Window* wd = &g_WindowData;
		//  the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		VkResult err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &g_DescriptorPool);
		check_vk_result(err);

		wd->Surface = VulkanSurface::Get().GetSurface();
		wd->ClearEnable = false;
		wd->Swapchain = VulkanSwapchain::Get().GetSwapChain();
		wd->Width = VulkanSwapchain::Get().GetExtent2D().width;
		wd->Height = VulkanSwapchain::Get().GetExtent2D().height;
		wd->ImageCount = (uint32_t)VulkanSwapchain::Get().GetImages().size();

		m_RenderPass = CreateRef<VulkanRenderPass>();
		m_RenderPass->Init(false /* ClearEnable */);

		wd->RenderPass = m_RenderPass->Get();
		wd->Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
		memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);

		for (uint32_t i = 0; i < wd->ImageCount; i++)
		{
			auto images = VulkanSwapchain::Get().GetImages();
			auto imageViews = VulkanSwapchain::Get().GetImageViews();
			wd->Frames[i].Backbuffer = images[i];
			wd->Frames[i].BackbufferView = imageViews[i];
			auto framebuffers = context->GetSwapchainFramebufferList();
			wd->Frames[i].Framebuffer = framebuffers[i].Get();
		}

		// 2: initialize imgui library

		// this initializes the core structures of imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.IniFilename = "assets/cache/imgui.ini";
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

		ImGui::StyleColorsDark();

		// this initializes imgui for GLFW
		ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanContext::GetInstance();
		init_info.PhysicalDevice = VulkanDevice::Get().GetPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = VulkanDevice::Get().GetPhysicalDeviceStruct().Indices.Graphics;
		init_info.Queue = VulkanDevice::Get().GetGraphicQueue();
		init_info.PipelineCache = VulkanDevice::Get().GetPipelineCache();
		init_info.DescriptorPool = g_DescriptorPool;
		init_info.Allocator = g_Allocator;
		init_info.CheckVkResultFn = VK_NULL_HANDLE;
		init_info.MinImageCount = 2;
		init_info.ImageCount = (uint32_t)VulkanSwapchain::Get().GetImages().size();
		init_info.RenderPass = m_RenderPass->Get();
		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void VulkanImGuiLayer::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiLayer::Render()
	{
		ImGui::Render();

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		DrawImgui(commandBuffer, m_RenderPass);

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGuiLayer::OnResize_Impl(uint32_t p_Width, uint32_t p_Height)
	{
		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());

		auto* wd = &g_WindowData;
		wd->Swapchain = VulkanSwapchain::Get().GetSwapChain();
		wd->Width = VulkanSwapchain::Get().GetExtent2D().width;
		wd->Height = VulkanSwapchain::Get().GetExtent2D().height;

		for (uint32_t i = 0; i < wd->ImageCount; i++)
		{
			auto images = VulkanSwapchain::Get().GetImages();
			auto imageViews = VulkanSwapchain::Get().GetImageViews();
			wd->Frames[i].Backbuffer = images[i];
			wd->Frames[i].BackbufferView = imageViews[i];
			auto framebuffers = context->GetSwapchainFramebufferList();
			wd->Frames[i].Framebuffer = framebuffers[i].Get();
		}
	}

	void VulkanImGuiLayer::Clear()
	{
		YM_CORE_TRACE("Destroying vulkan imgui impl...")
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}
