#include "YUME/yumepch.h"
#include "vulkan_command_buffer.h"
#include "Platform/Vulkan/Utils/vulkan_utils.h"
#include "Platform/Vulkan/Renderer/vulkan_swapchain.h"

#include "vulkan_device.h"
#include <Platform/Vulkan/Renderer/vulkan_renderpass.h>
#include <Platform/Vulkan/Renderer/vulkan_framebuffer.h>




namespace YUME
{
	VulkanCommandBuffer::VulkanCommandBuffer()
	{
	}

	VulkanCommandBuffer::VulkanCommandBuffer(const std::string& p_DebugName)
		: m_DebugName(p_DebugName)
	{
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		YM_PROFILE_FUNCTION()

		Free();
	}

	bool VulkanCommandBuffer::Init(RecordingLevel p_Level)
	{
		YM_PROFILE_FUNCTION()

		m_Level = p_Level;

		auto commandPool = VulkanDevice::Get().GetCommandPool();
		m_CommandPool = commandPool;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool		 = commandPool;
		allocInfo.level				 = (p_Level == RecordingLevel::PRIMARY) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;

		auto res					 = vkAllocateCommandBuffers(VulkanDevice::Get().GetDevice(), &allocInfo, &m_CommandBuffer);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		m_Semaphore					 = CreateRef<VulkanSemaphore>(SemaphoreType::None);
		m_Fence						 = CreateRef<VulkanFence>(false);
		m_Fence->Reset();

		if (!m_DebugName.empty())
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_COMMAND_BUFFER, m_DebugName.c_str(), m_CommandBuffer);

		return true;
	}

	bool VulkanCommandBuffer::Init(RecordingLevel p_Level, VkCommandPool p_CommandPool)
	{
		m_Level = p_Level;
		m_CommandPool = p_CommandPool;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext				 = VK_NULL_HANDLE;
		allocInfo.commandPool		 = p_CommandPool;
		allocInfo.level				 = (p_Level == RecordingLevel::PRIMARY) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;

		auto res					 = vkAllocateCommandBuffers(VulkanDevice::Get().GetDevice(), &allocInfo, &m_CommandBuffer);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		m_Semaphore					 = CreateRef<VulkanSemaphore>(SemaphoreType::None);
		m_Fence						 = CreateRef<VulkanFence>(true);

		if (!m_DebugName.empty())
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_COMMAND_BUFFER, m_DebugName.c_str(), m_CommandBuffer);

		return true;
	}

	void VulkanCommandBuffer::Begin()
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_Level == RecordingLevel::PRIMARY, "Begin() called from a secondary command buffer!");
		m_State = CommandBufferState::Recording;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = VK_NULL_HANDLE;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		auto res		= vkBeginCommandBuffer(m_CommandBuffer, &beginInfo);
		YM_CORE_ASSERT(res == VK_SUCCESS)
	}

	void VulkanCommandBuffer::BeginSecondary(const Ref<Pipeline>& p_Pipeline)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_Level == RecordingLevel::SECONDARY, "BeginSecondary() called from a primary command buffer!");
		m_State = CommandBufferState::Recording;

		const auto& renderPass			 = p_Pipeline->GetRenderPass();
		const auto& frameBuffer			 = p_Pipeline->GetFramebuffer();

		VkCommandBufferInheritanceInfo inheritanceInfo{};
		inheritanceInfo.sType			 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.subpass			 = 0;
		inheritanceInfo.renderPass		 = renderPass.As<VulkanRenderPass>()->Get();
		inheritanceInfo.framebuffer		 = frameBuffer.As<VulkanFramebuffer>()->Get();

		VkCommandBufferBeginInfo beginCreateInfo{};
		beginCreateInfo.sType			 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginCreateInfo.flags			 = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		beginCreateInfo.pInheritanceInfo = &inheritanceInfo;

		auto res = vkBeginCommandBuffer(m_CommandBuffer, &beginCreateInfo);
		YM_CORE_ASSERT(res == VK_SUCCESS)
	}

	void VulkanCommandBuffer::End()
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_State == CommandBufferState::Recording, "CommandBuffer ended before started recording")

		vkEndCommandBuffer(m_CommandBuffer);
		m_State = CommandBufferState::Ended;
	}

	bool VulkanCommandBuffer::Execute(VkPipelineStageFlags p_Flags, VkSemaphore p_WaitSemaphore, bool p_WaitFence)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_Level == RecordingLevel::PRIMARY, "Used Execute on secondary command buffer!");
		YM_CORE_ASSERT(m_State == CommandBufferState::Ended, "CommandBuffer executed before ended recording");

		uint32_t waitSemaphoreCount		= p_WaitSemaphore ? 1 : 0;
		uint32_t signalSemaphoreCount	= m_Semaphore ? 1 : 0;
		VkSemaphore semaphore			= m_Semaphore ? m_Semaphore->GetHandle() : VK_NULL_HANDLE;

		VkSubmitInfo submitInfo{};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext				= VK_NULL_HANDLE;
		submitInfo.waitSemaphoreCount   = 0;
		submitInfo.pWaitSemaphores		= nullptr;
		submitInfo.pWaitDstStageMask	= &p_Flags;
		submitInfo.commandBufferCount	= 1;
		submitInfo.pCommandBuffers		= &m_CommandBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphoreCount;
		submitInfo.pSignalSemaphores	= &semaphore;

		m_Fence->Reset();

		{
			YM_PROFILE_SCOPE("vkQueueSubmit");
			auto res = vkQueueSubmit(VulkanDevice::Get().GetGraphicQueue(), 1, &submitInfo, m_Fence->GetHandle());
			YM_CORE_VERIFY(res == VK_SUCCESS)
		}

		m_State = CommandBufferState::Submitted;
		return true;
	}

	void VulkanCommandBuffer::ExecuteSecondary(CommandBuffer* p_PrimaryCMD)
	{
		YM_PROFILE_FUNCTION();
		YM_CORE_ASSERT(m_Level != RecordingLevel::PRIMARY, "Used Execute on primary command buffer!");
		m_State = CommandBufferState::Submitted;

		vkCmdExecuteCommands(static_cast<VulkanCommandBuffer*>(p_PrimaryCMD)->GetHandle(), 1, &m_CommandBuffer);
	}

	void VulkanCommandBuffer::Submit()
	{
		YM_PROFILE_FUNCTION()
		Execute(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_NULL_HANDLE, true);
	}

	void VulkanCommandBuffer::Reset()
	{
		YM_PROFILE_FUNCTION()
		VKUtils::WaitIdle();

		if (m_State == CommandBufferState::Submitted)
			Wait();

		vkResetCommandBuffer(m_CommandBuffer, 0);
	}

	bool VulkanCommandBuffer::Flush()
	{
		YM_PROFILE_FUNCTION()

		if (m_State == CommandBufferState::Idle)
			return true;

		VKUtils::WaitIdle();

		if (m_State == CommandBufferState::Submitted)
			return Wait();

		return true;
	}

	bool VulkanCommandBuffer::Wait()
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_State == CommandBufferState::Submitted);

		m_Fence->WaitAndReset();
		m_State = CommandBufferState::Idle;

		return true;
	}

	void VulkanCommandBuffer::Free()
	{
		YM_PROFILE_FUNCTION()
		VKUtils::WaitIdle();

		if (m_State == CommandBufferState::Submitted)
			Wait();

		m_Fence = nullptr;
		m_Semaphore = nullptr;
		vkFreeCommandBuffers(VulkanDevice::Get().GetDevice(), m_CommandPool, 1, &m_CommandBuffer);
	}
}