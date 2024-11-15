#include "YUME/yumepch.h"
#include "vulkan_swapchain.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_context.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "vulkan_texture.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <YUME/Core/application.h>



namespace YUME
{
	VulkanSwapchain::~VulkanSwapchain()
	{
		VKUtils::WaitIdle();

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying swapchain frame data...")
		for (uint32_t i = 0; i < m_BufferCount; i++)
		{
			if (m_BufferCount >= MAX_SWAPCHAIN_BUFFERS)
				break;

			m_Frames[i].MainCommandBuffer->Reset();

			m_Frames[i].MainCommandBuffer.reset();
			m_Frames[i].CommandPool.reset();
			m_Frames[i].ImageAcquireSemaphore.reset();
		}

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying swapchain images view...")
		for (auto imageView : m_ImageViews)
		{
			vkDestroyImageView(VulkanDevice::Get().GetDevice(), imageView, VK_NULL_HANDLE);
		}

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying swapchain...")
		if (m_SwapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(VulkanDevice::Get().GetDevice(), m_SwapChain, VK_NULL_HANDLE);
		}

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying surface...")
		if (m_Surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(VulkanContext::GetInstance(), m_Surface, VK_NULL_HANDLE);
		}
	}

	void VulkanSwapchain::OnResize(uint32_t p_Width, uint32_t p_Height, bool p_Force, void* p_WindowHandle)
	{
		YM_PROFILE_FUNCTION()

		if (!p_Force && m_Extent2D.width == p_Width && m_Extent2D.height == p_Height)
			return;

		VKUtils::WaitIdle();

		m_OldSwapChain = m_SwapChain;
		m_SwapChain = VK_NULL_HANDLE;

		Init(m_Vsync, p_WindowHandle, { p_Width, p_Height });

		VKUtils::WaitIdle();
	}

	void VulkanSwapchain::Init(bool p_Vsync, void* p_Window, VkExtent2D p_Extent)
	{
		YM_PROFILE_FUNCTION()

		m_Vsync = p_Vsync;

		// Surface
		if (p_Window != nullptr && m_Surface == VK_NULL_HANDLE)
		{
			auto glfwWindow = (GLFWwindow*)p_Window;
			auto res = glfwCreateWindowSurface(VulkanContext::GetInstance(), glfwWindow, VK_NULL_HANDLE, &m_Surface);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}

		VkBool32 queueIndexSupported;
		auto res = vkGetPhysicalDeviceSurfaceSupportKHR(VulkanDevice::Get().GetPhysicalDevice(), VulkanDevice::Get().GetQueueFamilyIndices().Graphics, m_Surface, &queueIndexSupported);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_ASSERT(queueIndexSupported == VK_TRUE, "Present Queue not supported")

		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanDevice::Get().GetPhysicalDevice(), m_Surface, &surfaceCapabilities);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		uint32_t numPresentModes;
		res = vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanDevice::Get().GetPhysicalDevice(), m_Surface, &numPresentModes, VK_NULL_HANDLE);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		std::vector<VkPresentModeKHR> presentModes(numPresentModes);
		res = vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanDevice::Get().GetPhysicalDevice(), m_Surface, &numPresentModes, presentModes.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)

		ChooseSurfaceFormat();
		m_PresentMode = VKUtils::ChoosePresentMode(presentModes, p_Vsync);

		if (p_Window && (p_Extent.width == 0 || p_Extent.height == 0))
			ChooseSwapExtent2D(p_Window);
		else
			m_Extent2D = p_Extent;

		auto& device   = VulkanDevice::Get().GetDevice();

		m_BufferCount = surfaceCapabilities.maxImageCount;

		if (m_BufferCount > MAX_SWAPCHAIN_BUFFERS)
			m_BufferCount = MAX_SWAPCHAIN_BUFFERS;
		else if (m_BufferCount == 0)
			m_BufferCount = MAX_SWAPCHAIN_BUFFERS;

		YM_CORE_ASSERT(m_BufferCount > 1);

		VkSurfaceTransformFlagBitsKHR preTransform;
		if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		else
			preTransform = surfaceCapabilities.currentTransform;

		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType				 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface				 = m_Surface;
		createInfo.minImageCount		 = m_BufferCount;
		createInfo.imageFormat			 = m_Format.format;
		createInfo.imageColorSpace		 = m_Format.colorSpace;
		createInfo.imageExtent			 = m_Extent2D;
		createInfo.imageArrayLayers		 = 1;
		createInfo.imageUsage			 = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.imageSharingMode		 = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices	 = VK_NULL_HANDLE;
		createInfo.preTransform			 = preTransform;
		createInfo.compositeAlpha		 = compositeAlpha;
		createInfo.presentMode			 = m_PresentMode;
		createInfo.clipped				 = VK_TRUE;
		createInfo.oldSwapchain			 = m_OldSwapChain;

		if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
		{
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		res						   = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		if (m_OldSwapChain != VK_NULL_HANDLE)
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Destroying old swapchain...")

			for (uint32_t i = 0; i < m_BufferCount; i++)
			{
				if (i >= MAX_SWAPCHAIN_BUFFERS)
					break;

				if (m_Frames[i].MainCommandBuffer->GetState() == CommandBufferState::Submitted)
					m_Frames[i].MainCommandBuffer->Wait();

				m_Frames[i].MainCommandBuffer->Reset();

				m_Frames[i].ImageAcquireSemaphore.reset();
			}

			for (auto imageView : m_ImageViews)
			{
				vkDestroyImageView(device, imageView, VK_NULL_HANDLE);
			}

			m_ImageViews.clear();

			vkDestroySwapchainKHR(VulkanDevice::Get().GetDevice(), m_OldSwapChain, VK_NULL_HANDLE);
			m_OldSwapChain = VK_NULL_HANDLE;
		}

		std::vector<VkImage> images;

		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
		images.resize(imageCount);
		m_ImageViews.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, images.data());

		for (size_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo imgViewInfo{};
			imgViewInfo.sType							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgViewInfo.image							= images[i];
			imgViewInfo.viewType						= VK_IMAGE_VIEW_TYPE_2D;
			imgViewInfo.format							= m_Format.format;
			imgViewInfo.components.r					= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.components.g					= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.components.b					= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.components.a					= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			imgViewInfo.subresourceRange.baseMipLevel	= 0;
			imgViewInfo.subresourceRange.levelCount		= 1;
			imgViewInfo.subresourceRange.baseArrayLayer = 0;
			imgViewInfo.subresourceRange.layerCount		= 1;

			auto res			  = vkCreateImageView(device, &imgViewInfo, nullptr, &m_ImageViews[i]);
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}

		for (uint32_t i = 0; i < m_BufferCount; i++)
		{
			if (i >= MAX_SWAPCHAIN_BUFFERS)
				break;

			m_Buffers[i] = CreateRef<VulkanTexture2D>(images[i], m_ImageViews[i], m_Format.format, m_Extent2D.width, m_Extent2D.height);
		}

		CreateFrameData();
	}

	void VulkanSwapchain::AcquireNextImage()
	{
		YM_PROFILE_FUNCTION();

		static int FailedCount = 0;

		if (m_BufferCount == 1 && m_AcquireImageIndex != std::numeric_limits<uint32_t>::max())
			return;

		{
			YM_PROFILE_SCOPE("vkAcquireNextImageKHR")
			uint32_t imageIndex;
			auto& semaphore = m_Frames[m_CurrentBuffer].ImageAcquireSemaphore->GetHandle();
			auto result = vkAcquireNextImageKHR(VulkanDevice::Get().GetDevice(), m_SwapChain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			{
				YM_CORE_INFO("Acquire Image result : {}", result == VK_ERROR_OUT_OF_DATE_KHR ? "Out of Date" : "SubOptimal");

				if (result == VK_ERROR_OUT_OF_DATE_KHR)
				{
					OnResize(m_Extent2D.width, m_Extent2D.height, true);
				}
			}
			else if (result != VK_SUCCESS)
			{
				FailedCount++;
				YM_CORE_CRITICAL(VULKAN_PREFIX "Failed to acquire swap chain image!");

				YM_CORE_VERIFY(FailedCount <= 10, VULKAN_PREFIX "Failed to acquire swap chain image more than 10 times")

				return;
			}

			m_AcquireImageIndex = imageIndex;
			FailedCount = 0;
		}
	}

	void VulkanSwapchain::Present(const std::vector<VulkanSemaphore>& p_WaitSemaphores)
	{
		YM_PROFILE_FUNCTION();

		VkSemaphore vkWaitSemaphores[MAX_SWAPCHAIN_BUFFERS];
		uint32_t semaphoreCount = 0;

		for (auto semaphore : p_WaitSemaphores)
		{
			if (semaphoreCount >= MAX_SWAPCHAIN_BUFFERS)
				break;

			vkWaitSemaphores[semaphoreCount++] = semaphore.GetHandle();
		}

		if (p_WaitSemaphores.empty())
		{
			auto& frame			= GetCurrentFrameData();
			vkWaitSemaphores[0] = frame.MainCommandBuffer->GetSemaphore()->GetHandle();
			vkWaitSemaphores[1] = frame.ImageAcquireSemaphore->GetHandle();
			semaphoreCount		= 2;
		}

		VkPresentInfoKHR present{};
		present.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.pNext			   = VK_NULL_HANDLE;
		present.swapchainCount	   = 1;
		present.pSwapchains		   = &m_SwapChain;
		present.pImageIndices	   = &m_AcquireImageIndex;
		present.waitSemaphoreCount = semaphoreCount;
		present.pWaitSemaphores	   = vkWaitSemaphores;
		present.pResults		   = VK_NULL_HANDLE;

		{
			YM_PROFILE_SCOPE_T("SwapChain Present", Optick::Category::Wait)
			YM_PROFILE_GPU_FLIP(m_SwapChain)

			auto error = vkQueuePresentKHR(VulkanDevice::Get().GetGraphicQueue(), &present);

			if (error == VK_ERROR_OUT_OF_DATE_KHR)
			{
				YM_CORE_ERROR(VULKAN_PREFIX "SwapChain out of date");
			}
			else if (error == VK_SUBOPTIMAL_KHR)
			{
				YM_CORE_ERROR(VULKAN_PREFIX "SwapChain suboptimal");
			}
			else if (error != VK_SUCCESS)
			{
				YM_CORE_VERIFY(false)
			}
		}
	}

	void VulkanSwapchain::QueueSubmit()
	{
		YM_PROFILE_FUNCTION()

		auto& frame = GetCurrentFrameData();
		frame.MainCommandBuffer->Execute(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, frame.ImageAcquireSemaphore->GetHandle(), true);
	}

	void VulkanSwapchain::Begin()
	{
		YM_PROFILE_FUNCTION()

		m_CurrentBuffer = (m_CurrentBuffer + 1) % m_BufferCount;

		auto& commandBuffer = GetCurrentFrameData().MainCommandBuffer;
		if (commandBuffer->GetState() == CommandBufferState::Submitted)
		{
			if (!commandBuffer->Wait())
			{
				return;
			}
		}

		commandBuffer->Reset();
		AcquireNextImage();


		YM_PROFILE_GPU_CONTEXT(commandBuffer->GetHandle())
		commandBuffer->Begin();

		m_Buffers[m_CurrentBuffer].As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	void VulkanSwapchain::End()
	{
		YM_PROFILE_FUNCTION()

		auto& commandBuffer = GetCurrentFrameData().MainCommandBuffer;
		m_Buffers[m_CurrentBuffer].As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		commandBuffer->End();
		QueueSubmit();
	}

	void VulkanSwapchain::SetVsync(bool p_Enable)
	{
		if (p_Enable != m_Vsync) 
		{
			m_Vsync = p_Enable;
			OnResize(m_Extent2D.width, m_Extent2D.height, true);
		}
	}

	void VulkanSwapchain::ChooseSwapExtent2D(void* p_Window)
	{
		auto glfwWindow = (GLFWwindow*)p_Window;
		auto& device = VulkanDevice::Get().GetPhysicalDevice();
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &capabilities);

		VkExtent2D actualExtent{};

		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			actualExtent = capabilities.currentExtent;
		}
		else if (glfwWindow)
		{
			int width, height;
			glfwGetFramebufferSize(glfwWindow, &width, &height);
			
			actualExtent = {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height))
			};
		}
		else
		{
			YM_CORE_ASSERT(false)
		}

		m_Extent2D = actualExtent;
	}

	void VulkanSwapchain::ChooseSurfaceFormat()
	{
		YM_PROFILE_FUNCTION()

		VkPhysicalDevice physicalDevice = VulkanDevice::Get().GetPhysicalDevice();

		// Get list of supported surface formats
		uint32_t formatCount;
		auto res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, NULL);
		YM_CORE_VERIFY(res == VK_SUCCESS)
		YM_CORE_ASSERT(formatCount > 0)

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		res = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, surfaceFormats.data());
		YM_CORE_VERIFY(res == VK_SUCCESS)

		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_Format.format = VK_FORMAT_R8G8B8A8_SRGB;
			m_Format.colorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			bool found = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_R8G8B8A8_SRGB)
				{
					m_Format.format = surfaceFormat.format;
					m_Format.colorSpace = surfaceFormat.colorSpace;
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_Format.format = surfaceFormats[0].format;
				m_Format.colorSpace = surfaceFormats[0].colorSpace;
			}
		}
	}

	void VulkanSwapchain::CreateFrameData()
	{
		for (int i = 0; i < MAX_SWAPCHAIN_BUFFERS; i++)
		{
			m_Frames[i].ImageAcquireSemaphore = CreateUnique<VulkanSemaphore>(SemaphoreType::None);
			if (!m_Frames[i].MainCommandBuffer)
			{
				int graphic = VulkanDevice::Get().GetPhysicalDeviceStruct().Indices.Graphics;
				m_Frames[i].CommandPool = CreateUnique<VulkanCommandPool>(
					graphic,
					VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
					"SwapchainCommanPool Frame " + std::to_string(i)
				);
				m_Frames[i].MainCommandBuffer = CreateUnique<VulkanCommandBuffer>(
					"SwapchainCommandBuffer Frame " + std::to_string(i)
				);
				m_Frames[i].MainCommandBuffer->Init(RecordingLevel::PRIMARY, m_Frames[i].CommandPool->GetHandle());
			}
		}
	}
}