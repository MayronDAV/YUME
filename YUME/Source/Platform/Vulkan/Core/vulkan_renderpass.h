#pragma once
#include "YUME/Core/base.h"
#include "Platform/Vulkan/Renderer/vulkan_vertex_array.h"

// Lib
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>



namespace YUME
{
	class YM_API VulkanRenderPass
	{
		public:
			VulkanRenderPass() = default;
			~VulkanRenderPass();

			void Init();

			void Begin(VkCommandBuffer p_CommandBuffer, VkFramebuffer p_Frame, uint32_t p_Width, uint32_t p_Height, const glm::vec4& p_Color = {0.0f, 0.0f, 0.0f, 1.0f}, bool p_ClearDepth = false);
			void End(VkCommandBuffer p_CommandBuffer);

			VkRenderPass& Get() { return m_RenderPass; }

		private:
			VkRenderPass m_RenderPass = nullptr;
			
	};
}