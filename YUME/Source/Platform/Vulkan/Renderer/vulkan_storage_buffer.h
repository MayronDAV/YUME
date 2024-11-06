#pragma once
#include "YUME/Renderer/storage_buffer.h"
#include <Platform/Vulkan/Core/vulkan_memory_buffer.h>



namespace YUME
{
	class VulkanStorageBuffer : public StorageBuffer
	{
		public:
			VulkanStorageBuffer(size_t p_SizeBytes);
			~VulkanStorageBuffer() override = default;

			void SetData(void* p_Data, size_t p_SizeBytes, size_t p_Offset = 0ull) override;
			void Fill(uint32_t p_Data) override;
			void Fill(uint32_t p_Data, size_t p_SizeBytes) override;
			void Resize(size_t p_SizeBytes) override;

			size_t GetOffset() const override { return m_Offset; }
			size_t GetSize() const override { return m_Size; }
			VkBuffer GetBuffer() { return m_Buffer->GetBuffer(); }

		private:
			size_t m_Offset = 0ull;
			size_t m_Size = 0ull;

			Scope<VulkanMemoryBuffer> m_Buffer;
	};
}