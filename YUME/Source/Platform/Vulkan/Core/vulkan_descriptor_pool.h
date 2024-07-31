#pragma once
#include "YUME/Core/base.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class YM_API VulkanDescriptorPool
	{
		public:
			VulkanDescriptorPool() = default;
			~VulkanDescriptorPool() = default;

			void Init(uint32_t p_MaxSets, VkDescriptorPoolCreateFlags p_Flags);

			void Reset();

			void Destroy();

			VkDescriptorPool& Get() { return m_Handle; }

			explicit operator VkDescriptorPool() { return m_Handle; }
			explicit operator VkDescriptorPool&() { return m_Handle; }
			explicit operator const VkDescriptorPool& () const { return m_Handle; }

		private:
			VkDescriptorPool m_Handle;
	};
}