#pragma once
#include "YUME/Core/base.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class VulkanCommandBuffer
	{
		public:
			VulkanCommandBuffer() = default;
			~VulkanCommandBuffer();

			void Free();
			void Free(int p_Index);
			void Reset() const;
			void Reset(int p_Index);

			void Init(int p_Count = 1);

			void Begin(int p_Index = 0);
			void End(int p_Index = 0);

			VkCommandBuffer& Get(int p_Index = 0) { return m_CommandBuffers[p_Index]; }
			std::vector<VkCommandBuffer>& GetList() { return m_CommandBuffers; }
			int Count() const { return m_Count; }

		private:
			std::vector<VkCommandBuffer> m_CommandBuffers;
			int m_Count = 1;
	};
}