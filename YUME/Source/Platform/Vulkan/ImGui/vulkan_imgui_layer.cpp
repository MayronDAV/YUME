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
#include "Platform/Vulkan/Utils/vulkan_utils.h"

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
	static VkAllocationCallbacks* g_Allocator		   = VK_NULL_HANDLE;
	static VkDescriptorPool g_DescriptorPool		   = VK_NULL_HANDLE;
	static VkDescriptorSetLayout g_DescriptorSetLayout = VK_NULL_HANDLE;


	static void check_vk_result(VkResult p_Err)
	{
		if (p_Err == 0)
			return;
		printf("VkResult %d\n", p_Err);
		if (p_Err < 0)
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

	void DrawImgui(VkCommandBuffer p_Cmd, const Ref<VulkanRenderPass>& p_RenderPass, const Ref<VulkanFramebuffer>& p_Framebuffer)
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer;
		auto extent = VulkanSwapchain::Get().GetExtent2D();
		p_RenderPass->Begin(commandBuffer.get(), p_Framebuffer, extent.width, extent.height);
		{
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), p_Cmd);
		}
		p_RenderPass->End(commandBuffer.get());
	}

	VulkanImGuiLayer::~VulkanImGuiLayer()
	{
		YM_PROFILE_FUNCTION()

		VKUtils::WaitIdle();

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
		if (g_DescriptorSetLayout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(VulkanDevice::Get().GetDevice(), g_DescriptorSetLayout, VK_NULL_HANDLE);

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui descriptor pool...")
		if (g_DescriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(VulkanDevice::Get().GetDevice(), g_DescriptorPool, VK_NULL_HANDLE);
	}

	void VulkanImGuiLayer::OnAttach()
	{
		YM_PROFILE_FUNCTION()

		auto& device	= VulkanDevice::Get().GetDevice();
		auto glfwWindow = (GLFWwindow*)Application::Get().GetWindow().GetNativeWindow();

		ImGui_ImplVulkanH_Window* wd	  = &g_WindowData;
		//  the size of the pool is very oversize, but it's copied from imgui demo itself.
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,				 DESCRIPTOR_MAX_SAMPLERS				 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DESCRIPTOR_MAX_TEXTURES				 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,			 DESCRIPTOR_MAX_TEXTURES				 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,			 DESCRIPTOR_MAX_STORAGE_TEXTURES		 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,	 DESCRIPTOR_MAX_CONSTANT_BUFFERS		 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,	 DESCRIPTOR_MAX_STORAGE_BUFFERS			 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,		 DESCRIPTOR_MAX_CONSTANT_BUFFERS		 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,		 DESCRIPTOR_MAX_STORAGE_BUFFERS			 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, DESCRIPTOR_MAX_CONSTANT_BUFFERS_DYNAMIC },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, DESCRIPTOR_MAX_STORAGE_BUFFERS			 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,		 DESCRIPTOR_MAX_INPUT_ATTACHMENT		 }
		};

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets		= DESCRIPTOR_MAX_SETS * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes	= pool_sizes;
		VkResult err			= vkCreateDescriptorPool(device, &pool_info, g_Allocator, &g_DescriptorPool);
		check_vk_result(err);

		if (g_DescriptorSetLayout == VK_NULL_HANDLE)
		{
			VkDescriptorSetLayoutBinding binding[1] = {};
			binding[0].descriptorType				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			binding[0].descriptorCount				= 1;
			binding[0].stageFlags					= VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutCreateInfo info	= {};
			info.sType								= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount						= 1;
			info.pBindings							= binding;
			VkResult err							= vkCreateDescriptorSetLayout(device, &info, g_Allocator, &g_DescriptorSetLayout);
			check_vk_result(err);
		}

		wd->Surface		= VulkanSwapchain::Get().GetSurface();
		wd->ClearEnable = false;
		wd->Swapchain	= VulkanSwapchain::Get().GetSwapChain();
		wd->Width		= VulkanSwapchain::Get().GetExtent2D().width;
		wd->Height		= VulkanSwapchain::Get().GetExtent2D().height;
		wd->ImageCount  = (uint32_t)VulkanSwapchain::Get().GetBufferCount();

		TextureSpecification txSpec{};
		txSpec.Width			= wd->Width;
		txSpec.Height			= wd->Height;
		txSpec.Usage			= TextureUsage::TEXTURE_COLOR_ATTACHMENT;
		txSpec.GenerateMips		= false;
		txSpec.AnisotropyEnable = false;
		txSpec.DebugName		= "ImGuiTexture";

		RenderPassSpecification spec{};
		spec.Attachments.push_back(Texture2D::Create(txSpec));
		spec.ClearEnable		= true;
		spec.SwapchainTarget	= true;
		spec.DebugName			= "Imgui Renderpass";
		m_RenderPass			= CreateRef<VulkanRenderPass>(spec);

		auto buffers			= VulkanSwapchain::Get().GetBuffers();
		uint32_t bufferCount	= VulkanSwapchain::Get().GetBufferCount();

		for (uint32_t i = 0; i < bufferCount; i++)
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Creating imgui framebuffer...")

			FramebufferSpecification fbSpec{};
			fbSpec.Attachments.push_back(buffers[i]);
			fbSpec.RenderPass = m_RenderPass;
			fbSpec.Width	  = wd->Width;
			fbSpec.Height	  = wd->Height;
			fbSpec.DebugName  = "Imgui Framebuffer";

			m_Framebuffers.push_back(CreateRef<VulkanFramebuffer>(fbSpec));
		}

		wd->RenderPass  = m_RenderPass->Get();
		wd->Frames		= (ImGui_ImplVulkanH_Frame*)ImGui::MemAlloc(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
		memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);

		for (uint32_t i = 0; i < wd->ImageCount; i++)
		{
			auto vkBuffer				 = buffers[i].As<VulkanTexture2D>();
			wd->Frames[i].Backbuffer	 = vkBuffer->GetImage();
			wd->Frames[i].BackbufferView = vkBuffer->GetImageView();
			wd->Frames[i].Framebuffer	 = m_Framebuffers[i]->Get();
		}

		// 2: initialize imgui library

		// this initializes the core structures of imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io		= ImGui::GetIO(); (void)io;
		io.IniFilename  = "assets/imgui.ini";
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

		ImGui_ImplVulkan_InitInfo init_info{};
		init_info.Instance		  = VulkanContext::GetInstance();
		init_info.PhysicalDevice  = VulkanDevice::Get().GetPhysicalDevice();
		init_info.Device		  = device;
		init_info.QueueFamily	  = VulkanDevice::Get().GetPhysicalDeviceStruct().Indices.Graphics;
		init_info.Queue			  = VulkanDevice::Get().GetGraphicQueue();
		init_info.PipelineCache   = VulkanDevice::Get().GetPipelineCache();
		init_info.DescriptorPool  = g_DescriptorPool;
		init_info.Allocator		  = g_Allocator;
		init_info.CheckVkResultFn = check_vk_result;
		init_info.MinImageCount   = VulkanSwapchain::Get().GetBufferCount();
		init_info.ImageCount	  = VulkanSwapchain::Get().GetBufferCount();
		init_info.RenderPass	  = m_RenderPass->Get();
		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void VulkanImGuiLayer::OnDetach()
	{
		YM_PROFILE_FUNCTION()

		VKUtils::WaitIdle();

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying imgui...")
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanImGuiLayer::OnEvent(Event& p_Event)
	{
		YM_PROFILE_FUNCTION()

		if (m_BlockEvents)
		{
			YM_PROFILE_SCOPE("Imgui Events")

			const ImGuiIO& io = ImGui::GetIO();
			p_Event.Handled  |= p_Event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			p_Event.Handled  |= p_Event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void VulkanImGuiLayer::Begin()
	{
		YM_PROFILE_FUNCTION()

		DeleteUnusedTextures();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiLayer::End()
	{
		YM_PROFILE_FUNCTION()

		ImGui::Render();

		auto& commandBuffer = VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer->GetHandle();
		auto currentFrame	= VulkanSwapchain::Get().GetCurrentBuffer();
		auto buffers		= VulkanSwapchain::Get().GetBuffers();

		auto width			= buffers[currentFrame]->GetWidth();
		auto height			= buffers[currentFrame]->GetHeight();

		if (m_Framebuffers[currentFrame]->GetWidth() != width ||
			m_Framebuffers[currentFrame]->GetHeight() != height)
		{
			YM_CORE_INFO(VULKAN_PREFIX "Resizing imgui...")

			auto* wd	  = &g_WindowData;
			wd->Swapchain = VulkanSwapchain::Get().GetSwapChain();
			wd->Width	  = width;
			wd->Height	  = height;

			uint32_t bufferCount = VulkanSwapchain::Get().GetBufferCount();

			for (uint32_t i = 0; i < bufferCount; i++)
			{
				std::vector<Ref<Texture2D>> attachments;
				attachments.push_back(buffers[i]);

				m_Framebuffers[i]->OnResize(width, height, attachments);
			}

			wd->ImageCount = bufferCount;
			for (uint32_t i = 0; i < wd->ImageCount; i++)
			{
				auto vkBuffer = buffers[i].As<VulkanTexture2D>();

				wd->Frames[i].Backbuffer	 = vkBuffer->GetImage();
				wd->Frames[i].BackbufferView = vkBuffer->GetImageView();
				wd->Frames[i].Framebuffer	 = m_Framebuffers[i]->Get();
			}
		}

		DrawImgui(commandBuffer, m_RenderPass, m_Framebuffers[VulkanSwapchain::Get().GetImageIndex()]);

		ImGuiIO& io		 = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize	 = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	ImTextureID VulkanImGuiLayer::AddTexture(const Ref<Texture2D>& p_Texture) const
	{
		YM_PROFILE_FUNCTION()

		auto device = VulkanDevice::Get().GetDevice();
		auto vkTexture = p_Texture.As<VulkanTexture2D>();
		auto spec = vkTexture->GetSpecification();
		auto commandBuffer = VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer.get();

		if (spec.Usage == TextureUsage::TEXTURE_COLOR_ATTACHMENT || spec.Usage == TextureUsage::TEXTURE_SAMPLED)
		{
			vkTexture->TransitionImage(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
		}
		else if (spec.Usage == TextureUsage::TEXTURE_DEPTH_STENCIL_ATTACHMENT)
		{
			vkTexture->TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, commandBuffer);
		}
		
		auto sampler	  = vkTexture->GetImageSampler();
		auto image_view	  = vkTexture->GetImageView();
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
			alloc_info.sType			  = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool	  = g_DescriptorPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts		  = &g_DescriptorSetLayout;
			vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set);

			s_ImGuiTextures[hash] = { descriptor_set, (float)Clock::GetTime() };
		}

		// Update the Descriptor Set:
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler				= sampler;
			desc_image[0].imageView				= image_view;
			desc_image[0].imageLayout			= image_layout;

			VkWriteDescriptorSet  write_desc[1] = {};
			write_desc[0].sType					= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet				= descriptor_set;
			write_desc[0].descriptorCount		= 1;
			write_desc[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo			= desc_image;
			vkUpdateDescriptorSets(device, 1, write_desc, 0, nullptr);
		}

		return (ImTextureID)descriptor_set;
	}
}
