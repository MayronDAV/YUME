#pragma once
#include "YUME/Core/base.h"
#include "YUME/Core/command_buffer.h"
#include "vulkan_sync.h"
#include "YUME/Renderer/pipeline.h"

// Lib
#include <vulkan/vulkan.h>



namespace YUME
{
	class VulkanCommandBuffer : public CommandBuffer
	{
		public:
			VulkanCommandBuffer();
			VulkanCommandBuffer(const std::string& p_DebugName);
			~VulkanCommandBuffer() override;

			bool Init(RecordingLevel p_Level) override;
			bool Init(RecordingLevel p_Level, VkCommandPool p_CommandPool);

			void Begin() override;
			void BeginSecondary(const Ref<Pipeline>& p_Pipeline) override;
			void End() override;

			bool Execute(VkPipelineStageFlags p_Flags, VkSemaphore p_WaitSemaphore, bool p_WaitFence = false);
			void ExecuteSecondary(CommandBuffer* p_PrimaryCMD) override;
			void Submit() override;

			bool Wait() override;
			void Reset() override;
			void Free() override;
			bool Flush() override;

			VkCommandBuffer& GetHandle() { return m_CommandBuffer; }
			RecordingLevel GetRecordingLevel() const override { return m_Level; }
			CommandBufferState GetState() const override { return m_State; }

			const Ref<VulkanFence>& GetFence() const { return m_Fence; }
			const Ref<VulkanSemaphore>& GetSemaphore() const { return m_Semaphore; }

		private:
			VkCommandPool m_CommandPool = VK_NULL_HANDLE;

			RecordingLevel m_Level = RecordingLevel::PRIMARY;
			CommandBufferState m_State = CommandBufferState::Idle;
			VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

			Ref<VulkanFence> m_Fence = nullptr;
			Ref<VulkanSemaphore> m_Semaphore = nullptr;
			std::string m_DebugName;
	};
}