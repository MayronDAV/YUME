#include "YUME/yumepch.h"
#include "vulkan_swapchain.h"
#include "vulkan_device.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>



namespace YUME
{
	VulkanSwapchain::~VulkanSwapchain()
	{
		CleanUp();
	}

	void VulkanSwapchain::CleanUp()
	{
		auto device = VulkanDevice::Get().GetDevice();

		YM_CORE_TRACE("Destroying vulkan image views...")
		for (auto imageView : m_ImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		YM_CORE_TRACE("Destroying vulkan swapchain...")
		if (m_SwapChain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(device, m_SwapChain, VK_NULL_HANDLE);
	}

	void VulkanSwapchain::Invalidade(uint32_t p_Width, uint32_t p_Height)
	{
		CleanUp();

		Init(m_Vsync, m_Window, { p_Width, p_Height });
	}

	void VulkanSwapchain::Init(bool p_Vsync, void* p_Window, VkExtent2D p_Extent)
	{
		m_Vsync = p_Vsync;
		m_Window = p_Window;

		CheckSurfaceFormat();
		ChoosePresentMode(p_Vsync);

		if (p_Extent.width == 0 || p_Extent.height == 0)
			ChooseSwapExtent2D(p_Window);
		else
			m_Extent2D = p_Extent;

		auto& device = VulkanDevice::Get().GetDevice();
		const auto& physDevice = VulkanDevice::Get().GetPhysicalDeviceStruct();
		auto& capabilities = physDevice.SurfaceCapabilities;

		uint32_t imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
			imageCount = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanSurface::Get().GetSurface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = m_Format.format;
		createInfo.imageColorSpace = m_Format.colorSpace;
		createInfo.imageExtent = m_Extent2D;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	
		if (auto& index = physDevice.Indices;
			index.Graphics == index.Present)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = VK_NULL_HANDLE; // Optional
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			std::vector<int> indices = { index.Graphics, index.Present };
			createInfo.pQueueFamilyIndices = (uint32_t*)indices.data();
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_PresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		auto res = vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_SwapChain);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		imageCount = 0;
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, nullptr);
		m_Images.resize(imageCount);
		m_ImageViews.resize(imageCount);
		vkGetSwapchainImagesKHR(device, m_SwapChain, &imageCount, m_Images.data());

		for (size_t i = 0; i < imageCount; i++)
		{
			VkImageViewCreateInfo imgViewInfo{};
			imgViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgViewInfo.image = m_Images[i];
			imgViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imgViewInfo.format = m_Format.format;
			imgViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imgViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imgViewInfo.subresourceRange.baseMipLevel = 0;
			imgViewInfo.subresourceRange.levelCount = 1;
			imgViewInfo.subresourceRange.baseArrayLayer = 0;
			imgViewInfo.subresourceRange.layerCount = 1;

			res = vkCreateImageView(device, &imgViewInfo, nullptr, &m_ImageViews[i]);
			YM_CORE_ASSERT(res == VK_SUCCESS)

		}
	}

	void VulkanSwapchain::ChooseSwapExtent2D(void* p_Window)
	{
		auto glfwWindow = (GLFWwindow*)p_Window;
		auto& device = VulkanDevice::Get().GetPhysicalDevice();
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, VulkanSurface::Get().GetSurface(), &capabilities);

		VkExtent2D actualExtent;

		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			actualExtent = capabilities.currentExtent;
		}
		else 
		{
			int width, height;
			glfwGetFramebufferSize(glfwWindow, &width, &height);
			
			actualExtent = {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)height))
			};
		}

		m_Extent2D = actualExtent;
	}

	void VulkanSwapchain::ChoosePresentMode(bool p_Vsync)
	{
		if (p_Vsync)
		{		
			for (const auto& physDevice = VulkanDevice::Get().GetPhysicalDeviceStruct();
				 const auto& availablePresentMode : physDevice.PresentModes)
			{
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					YM_CORE_INFO("Choosing present mode - VK_PRESENT_MODE_MAILBOX_KHR")
					m_PresentMode = availablePresentMode;
					return;
				}
			}

			YM_CORE_INFO("Choosing present mode - VK_PRESENT_MODE_FIFO_KHR")
			m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
			return;
		}

		YM_CORE_INFO("Choosing present mode - VK_PRESENT_MODE_IMMEDIATE_KHR")
		m_PresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	void VulkanSwapchain::CheckSurfaceFormat()
	{
		const auto& surfaceFormats = VulkanDevice::Get().GetPhysicalDeviceStruct().SurfaceFormats;

		for (const auto& availableFormat : surfaceFormats) 
		{
			if (availableFormat.format == m_Format.format && availableFormat.colorSpace == m_Format.colorSpace)
				return;
		}

		YM_CORE_WARN("Format not found [ VK_FORMAT_R8G8B8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR ], using the first available surface format...")
		m_Format = surfaceFormats[0];
	}
}