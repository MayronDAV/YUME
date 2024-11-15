#pragma once
#include "YUME/Core/base.h"
#include <vulkan/vulkan.h>


namespace YUME
{
	class VulkanFence
	{
        public:
            VulkanFence(bool p_CreateSignaled = false);
            ~VulkanFence();

            bool CheckState();
            bool IsSignaled();
            VkFence& GetHandle() { return m_Handle; }

            bool Wait(uint64_t p_TimeoutNanoseconds = UINT64_MAX);
            void Reset();
            void WaitAndReset();

        private:
            VkFence m_Handle = VK_NULL_HANDLE;
            bool m_Signaled = false;
	};

    enum class SemaphoreType
    {
        None = 0,
        Timeline,
        Binary
    };

    class VulkanSemaphore
    {
        public:
            VulkanSemaphore(SemaphoreType p_Type = SemaphoreType::None);
            ~VulkanSemaphore();

            VkSemaphore& GetHandle() { return m_Handle; }

            void Signal(uint64_t p_Value);
            void Wait(uint64_t p_Value, uint64_t p_Timeout);

            uint64_t GetValue();

        private:
            VkSemaphore m_Handle = VK_NULL_HANDLE;
            SemaphoreType m_Type = SemaphoreType::None;
    };


} // YUME