#pragma once
#include "YUME/Core/base.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class YM_API VulkanMemoryBuffer
	{
		public:
            VulkanMemoryBuffer() = default;
            VulkanMemoryBuffer(VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_MemoryProperyFlags, VkDeviceSize p_SizeBytes);
            virtual ~VulkanMemoryBuffer();

            void Init(VkBufferUsageFlags p_Usage, VkMemoryPropertyFlags p_MemoryProperyFlags, VkDeviceSize p_SizeBytes);
            void Resize(VkDeviceSize p_SizeBytes, const void* p_Data);
            void SetData(VkDeviceSize p_SizeBytes, const void* p_Data, VkDeviceSize p_Offset = 0, bool p_AddBarrier = false);
            const VkBuffer& GetBuffer() const { return m_Buffer; }

            void Map(VkDeviceSize p_SizeBytes = VK_WHOLE_SIZE, VkDeviceSize p_Offset = 0);
            void UnMap();
            void* GetMapped() { return m_Mapped; }
            void Flush(VkDeviceSize p_SizeBytes = VK_WHOLE_SIZE, VkDeviceSize p_Offset = 0);
            void Invalidate(VkDeviceSize p_SizeBytes = VK_WHOLE_SIZE, VkDeviceSize p_Offset = 0);
            void Destroy(bool p_DeletionQueue = false) noexcept;
            void SetUsage(VkBufferUsageFlags p_Flags) { m_UsageFlags = p_Flags; }
            void SetMemoryProperyFlags(VkBufferUsageFlags p_Flags) { m_MemoryPropertyFlags = p_Flags; }
            void SetDeleteWithoutQueue(bool p_Value) { m_DeleteWithoutQueue = p_Value; }

		private:
            VkBuffer m_Buffer{};
            VkDeviceMemory m_Memory{};
			VkMemoryRequirements m_Requirements{};
            VkDeviceSize m_SizeBytes = 0;
            VkBufferUsageFlags m_UsageFlags;
            VkMemoryPropertyFlags m_MemoryPropertyFlags;
            void* m_Mapped = nullptr;
            bool m_DeleteWithoutQueue = false;

	};
}