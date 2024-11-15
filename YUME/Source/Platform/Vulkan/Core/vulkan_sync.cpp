#include "YUME/yumepch.h"
#include "vulkan_sync.h"
#include "vulkan_device.h"



namespace YUME
{
	VulkanFence::VulkanFence(bool p_CreateSignaled)
	{
		YM_PROFILE_FUNCTION()

		m_Signaled = p_CreateSignaled;

		VkFenceCreateInfo fenceCreateInfo{};
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.flags = p_CreateSignaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		vkCreateFence(VulkanDevice::Get().GetDevice(), &fenceCreateInfo, nullptr, &m_Handle);
	}

	VulkanFence::~VulkanFence()
	{
		YM_PROFILE_FUNCTION()
		vkDestroyFence(VulkanDevice::Get().GetDevice(), m_Handle, nullptr);
	}

	bool VulkanFence::CheckState()
	{
		YM_CORE_ASSERT(!m_Signaled, "Fence Signaled");

		const VkResult result = vkGetFenceStatus(VulkanDevice::Get().GetDevice(), m_Handle);
		if (result == VK_SUCCESS)
		{
			m_Signaled = true;
			return true;
		}

		return false;
	}
	bool VulkanFence::IsSignaled()
	{
		YM_PROFILE_FUNCTION()

		if (m_Signaled)
			return true;
		else
		{
			return CheckState();
		}
	}
	bool VulkanFence::Wait(uint64_t p_TimeoutNanoseconds)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(!m_Signaled, "Fence Signaled");

		const VkResult result = vkWaitForFences(VulkanDevice::Get().GetDevice(), 1, &m_Handle, VK_TRUE, p_TimeoutNanoseconds);
		if (result == VK_SUCCESS)
		{
			m_Signaled = true;
			return true;
		}

		return false;
	}

	void VulkanFence::Reset()
	{
		YM_PROFILE_FUNCTION()

		if (m_Signaled)
			vkResetFences(VulkanDevice::Get().GetDevice(), 1, &m_Handle);

		m_Signaled = false;
	}

	void VulkanFence::WaitAndReset()
	{
		YM_PROFILE_FUNCTION()
		if (!IsSignaled())
			Wait();

		Reset();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////


	VulkanSemaphore::VulkanSemaphore(SemaphoreType p_Type)
	{
		YM_PROFILE_FUNCTION()
		m_Type = p_Type;

		VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo = {};
		semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		semaphoreTypeCreateInfo.pNext = nullptr;
		semaphoreTypeCreateInfo.semaphoreType = (p_Type == SemaphoreType::Timeline) ? VK_SEMAPHORE_TYPE_TIMELINE : VK_SEMAPHORE_TYPE_BINARY;
		semaphoreTypeCreateInfo.initialValue = 0;

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = (p_Type != SemaphoreType::None) ? &semaphoreTypeCreateInfo : VK_NULL_HANDLE;

		vkCreateSemaphore(VulkanDevice::Get().GetDevice(), &semaphoreInfo, VK_NULL_HANDLE, &m_Handle);
	}

	VulkanSemaphore::~VulkanSemaphore()
	{
		YM_PROFILE_FUNCTION()
		vkDestroySemaphore(VulkanDevice::Get().GetDevice(), m_Handle, nullptr);
	}

	void VulkanSemaphore::Signal(uint64_t p_Value)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_Type == SemaphoreType::Timeline)

		VkSemaphoreSignalInfo semaphoreSignalInfo = {};
		semaphoreSignalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		semaphoreSignalInfo.pNext = nullptr;
		semaphoreSignalInfo.semaphore = m_Handle;
		semaphoreSignalInfo.value = p_Value;

		auto res = vkSignalSemaphore(VulkanDevice::Get().GetDevice(), &semaphoreSignalInfo);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

	void VulkanSemaphore::Wait(uint64_t p_Value, uint64_t p_Timeout)
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_Type == SemaphoreType::Timeline)

		VkSemaphoreWaitInfo semaphoreWaitInfo = {};
		semaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		semaphoreWaitInfo.pNext = nullptr;
		semaphoreWaitInfo.flags = 0;
		semaphoreWaitInfo.semaphoreCount = 1;
		semaphoreWaitInfo.pSemaphores = &m_Handle;
		semaphoreWaitInfo.pValues = &p_Value;

		auto res = vkWaitSemaphores(VulkanDevice::Get().GetDevice(), &semaphoreWaitInfo, p_Timeout);
		YM_CORE_ASSERT(res == VK_SUCCESS, "Failed to wait for semaphore")
	}

	uint64_t VulkanSemaphore::GetValue()
	{
		YM_PROFILE_FUNCTION()
		YM_CORE_ASSERT(m_Type == SemaphoreType::Timeline)

		uint64_t value = 0;
		auto res = vkGetSemaphoreCounterValue(VulkanDevice::Get().GetDevice(), m_Handle, &value);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		return value;
	}
}
