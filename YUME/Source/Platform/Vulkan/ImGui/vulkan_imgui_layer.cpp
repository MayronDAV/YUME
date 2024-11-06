#include "YUME/yumepch.h"
#include "vulkan_imgui_layer.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "Platform/Vulkan/Renderer/vulkan_context.h"
#include "YUME/Core/application.h"
#include "Platform/Vulkan/Core/vulkan_window.h"
#include "YUME/Renderer/texture.h"
#include "YUME/Utils/hash_combiner.h"
#include "Platform/Vulkan/Renderer/vulkan_texture.h"
#include "YUME/Utils/clock.h"

// Lib
#include <vulkan/vulkan.h>
#include <imgui/imgui.h>
#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>




namespace YUME
{
	static ImGui_ImplVulkanH_Window g_WindowData;
	static VkAllocationCallbacks* g_Allocator = nullptr;
	static VkDescriptorPool g_DescriptorPool = nullptr;
	static VkDescriptorSetLayout g_DescriptorSetLayout = nullptr;


	static void check_vk_result(VkResult err)
	{
		if (err == 0)
			return;
		printf("VkResult %d\n", err);
		if (err < 0)
			abort();
	}

	#define MAX_IMGUI_TEXTURES 1000;
	struct ImGuiTextures
	{
		VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
		float TimeSinceLastAccessed;
	};
	static std::unordered_map<uint64_t, ImGuiTextures> s_ImGuiTextures;
	static const float s_TextureLifeTime = 1.0f;

	static void ClearTextures()
	{
		YM_PROFILE_FUNCTION()

		for (auto&& [key, value] : s_ImGuiTextures)
		{
			if (value.DescriptorSet)
			{
				vkFreeDescriptorSets(VulkanDevice::Get().GetDevice(), g_DescriptorPool, 1, &value.DescriptorSet);
				value.DescriptorSet = VK_NULL_HANDLE;
			}
		}

		s_ImGuiTextures.clear();
	}

	static void DeleteUnusedTextures()
	{
		YM_PROFILE_FUNCTION()

		static std::size_t keysToDelete[256];
		std::size_t keysToDeleteCount = 0;

		for (auto&& [key, value] : s_ImGuiTextures)
		{
			if (value.DescriptorSet && (Clock::GetTime() - value.TimeSinceLastAccessed) > s_TextureLifeTime)
			{
				keysToDelete[keysToDeleteCount] = key;
				keysToDeleteCount++;
			}

			if (keysToDeleteCount >= 256)
				break;
		}

		for (std::size_t i = 0; i < keysToDeleteCount; i++)
		{
			vkFreeDescriptorSets(VulkanDevice::Get().GetDevice(), g_DescriptorPool, 1, &s_ImGuiTextures[keysToDelete[i]].DescriptorSet);
			s_ImGuiTextures[keysToDelete[i]].DescriptorSet = VK_NULL_HANDLE;
			s_ImGuiTextures.erase(keysToDelete[i]);
		}
	}

	void DrawImgui(VkCommandBuffer p_Cmd, const Ref<VulkanRenderPass>& p_RenderPass, const Ref<VulkanRenderPassFramebuffer>& p_Framebuffer)
	{
		YM_PROFILE_FUNCTION()

		auto extent = VulkanSwapchain::Get().GetExtent2D();
		p_RenderPass->Begin(p_Framebuffer, extent.width, extent.height);
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), p_Cmd);
		}
		p_RenderPass->End();
	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{
		YM_PROFILE_FUNCTION()

		vkDeviceWaitIdle(VulkanDevice::Get().GetDevice());

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui framebuffers...")
		for (auto framebuffer : m_Framebuffers)
		{
			VulkanContext::PushFunction([framebuffer]()
				{
					framebuffer->CleanUp();
				});
		}

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui render pass...")
		auto renderpass = m_RenderPass;
		VulkanContext::PushFunction([renderpass]()
		{
			renderpass->CleanUp();
		});


		ClearTextures();


		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui descriptor set layout...")
		if (g_DescriptorSetLayout)
			vkDestroyDescriptorSetLayout(VulkanDevice::Get().GetDevice(), g_DescriptorSetLayout, VK_NULL_HANDLE);

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui descriptor pool...")
		if (g_DescriptorPool)
			vkDestroyDescriptorPool(VulkanDevice::Get().GetDevice(), g_DescriptorPool, VK_NULL_HANDLE);
	}

	ImTextureID VulkanImGuiLayer::AddTexture(const Ref<Texture2D>& p_Texture) const
	{
		YM_PROFILE_FUNCTION()

		auto device = VulkanDevice::Get().GetDevice();
		auto vkTexture = p_Texture.As<VulkanTexture2D>();
		auto spec = vkTexture->GetSpecification();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED)
		{
			vkTexture->TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
		{
			vkTexture->TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);
		}
		
		auto sampler = vkTexture->GetImageSampler();
		auto image_view = vkTexture->GetImageView();
		auto image_layout = vkTexture->GetLayout();

		uint64_t hash = 0;
		HashCombine(hash, sampler, image_view, (int)image_layout);

		VkDescriptorSet descriptor_set;

		auto found = s_ImGuiTextures.find(hash);
		if (found != s_ImGuiTextures.end() && found->second.DescriptorSet)
		{
			found->second.TimeSinceLastAccessed = (float)Clock::GetTime();
			descriptor_set = found->second.DescriptorSet;
		}
		else
		{
			// Create the Descriptor Set:

			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = g_DescriptorPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &g_DescriptorSetLayout;
			vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set);

			s_ImGuiTextures[hash] = { descriptor_set, (float)Clock::GetTime() };
		}

		// Update the Descriptor Set:
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = sampler;
			desc_image[0].imageView = image_view;
			desc_image[0].imageLayout = image_layout;
			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = descriptor_set;
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(device, 1, write_desc, 0, nullptr);
		}

		return (ImTextureID)descriptor_set;
	}

	void VulkanImGuiLayer::Init()
	{
		YM_PROFILE_FUNCTION()

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

		{
			VkDescriptorSetLayoutBinding binding[1] = {};
			binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding[0].descriptorCount = 1;
			binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			VkDescriptorSetLayoutCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = 1;
			info.pBindings = binding;
			VkResult err = vkCreateDescriptorSetLayout(device, &info, nullptr, &g_DescriptorSetLayout);
			check_vk_result(err);
		}

		wd->Surface = VulkanSurface::Get().GetSurface();
		wd->ClearEnable = false;
		wd->Swapchain = VulkanSwapchain::Get().GetSwapChain();
		wd->Width = VulkanSwapchain::Get().GetExtent2D().width;
		wd->Height = VulkanSwapchain::Get().GetExtent2D().height;
		wd->ImageCount = (uint32_t)VulkanSwapchain::Get().GetImages().size();

		TextureSpecification txSpec{};
		txSpec.Width = wd->Width;
		txSpec.Height = wd->Height;
		txSpec.Usage = TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		txSpec.GenerateMips = false;

		RenderPassSpecification spec{};
		spec.Attachments.push_back(CreateRef<VulkanTexture2D>(txSpec));
		spec.ClearEnable = true;
		spec.SwapchainTarget = true;
		m_RenderPass = CreateRef<VulkanRenderPass>(spec);

		auto& images = VulkanSwapchain::Get().GetImages();
		auto& imageViews = VulkanSwapchain::Get().GetImageViews();

		for (size_t i = 0; i < images.size(); i++)
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Creating imgui framebuffer...")

			RenderPassFramebufferSpec fbSpec{};
			fbSpec.Attachments.push_back(CreateRef<VulkanTexture2D>(images[i], imageViews[i],
				VulkanSwapchain::Get().GetFormat().format, wd->Width, wd->Height));
			fbSpec.RenderPass = m_RenderPass;
			fbSpec.Width = wd->Width;
			fbSpec.Height = wd->Height;

			m_Framebuffers.push_back(CreateRef<VulkanRenderPassFramebuffer>(fbSpec));
		}

		wd->RenderPass = m_RenderPass->Get();
		wd->Frames = (ImGui_ImplVulkanH_Frame*)ImGui::MemAlloc(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
		memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);

		for (uint32_t i = 0; i < wd->ImageCount; i++)
		{
			wd->Frames[i].Backbuffer = images[i];
			wd->Frames[i].BackbufferView = imageViews[i];
			wd->Frames[i].Framebuffer = m_Framebuffers[i]->Get();
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
		YM_PROFILE_FUNCTION()

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiLayer::Render()
	{
		YM_PROFILE_FUNCTION()

		ImGui::Render();

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();

		DrawImgui(commandBuffer, m_RenderPass, m_Framebuffers[context->GetCurrentImageIndex()]);

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		DeleteUnusedTextures();
	}

	void VulkanImGuiLayer::OnResize_Impl(uint32_t p_Width, uint32_t p_Height)
	{
		YM_PROFILE_FUNCTION()

		auto* wd = &g_WindowData;
		wd->Swapchain = VulkanSwapchain::Get().GetSwapChain();
		wd->Width = VulkanSwapchain::Get().GetExtent2D().width;
		wd->Height = VulkanSwapchain::Get().GetExtent2D().height;

		auto& images = VulkanSwapchain::Get().GetImages();
		auto& imageViews = VulkanSwapchain::Get().GetImageViews();

		for (size_t i = 0; i < images.size(); i++)
		{
			std::vector<Ref<Texture2D>> attachments;
			attachments.push_back(CreateRef<VulkanTexture2D>(images[i], imageViews[i],
				VulkanSwapchain::Get().GetFormat().format, wd->Width, wd->Height));

			m_Framebuffers[i]->OnResize(wd->Width, wd->Height, attachments);
		}

		for (uint32_t i = 0; i < wd->ImageCount; i++)
		{
			wd->Frames[i].Backbuffer = images[i];
			wd->Frames[i].BackbufferView = imageViews[i];
			wd->Frames[i].Framebuffer = m_Framebuffers[i]->Get();
		}
	}

	void VulkanImGuiLayer::Clear()
	{
		YM_PROFILE_FUNCTION()

		vkDeviceWaitIdle(VulkanDevice::Get().GetDevice());

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui...")
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}
