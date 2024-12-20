#pragma once

#include "YUME/Core/base.h"

#include <vulkan/vulkan.h>



namespace YUME
{
	class VulkanCommandPool
	{
        public:
            VulkanCommandPool(uint32_t p_QueueIndex, VkCommandPoolCreateFlags p_Flags, const std::string& p_DebugName = "VkCommandPool");
            ~VulkanCommandPool();

            void Reset();

            const VkCommandPool& GetHandle() const { return m_Handle; }

        private:
            VkCommandPool m_Handle;
	};
}