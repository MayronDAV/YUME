#include "YUME/yumepch.h"
#include "vulkan_pipeline.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_context.h"
#include "vulkan_shader.h"

#include "YUME/Core/application.h"



namespace YUME
{
	VulkanPipeline::VulkanPipeline(const PipelineCreateInfo& p_CreateInfo)
	{
		Init(p_CreateInfo);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		YM_CORE_TRACE("Destroying vulkan vertex array...")

		for (auto& buffer : m_VertexArrays)
		{
			buffer.reset();
		}

		CleanUp();
	}

	void VulkanPipeline::CleanUp()
	{
		YM_CORE_TRACE("Destroying vulkan pipeline...")
		if (m_Pipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(VulkanDevice::Get().GetDevice(), m_Pipeline, VK_NULL_HANDLE);
	}

	void VulkanPipeline::Init(const PipelineCreateInfo& p_CreateInfo)
	{
		YM_CORE_VERIFY(p_CreateInfo.Shader != nullptr)

		m_Context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		m_CreateInfo = p_CreateInfo;
		m_Shader = p_CreateInfo.Shader;

		size_t totalBindingSize = 0;
		size_t totalInputAttribSize = 0;

		for (size_t i = 0; i < m_VertexArrays.size(); i++) 
		{
			auto vkVAO = dynamic_cast<VulkanVertexArray*>(m_VertexArrays[i].get());
			totalBindingSize += vkVAO->GetBindingDescription().size();
			totalInputAttribSize += vkVAO->GetAttributeDescription().size();
		}

		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		bindingDescriptions.reserve(totalBindingSize);
		attributeDescriptions.reserve(totalInputAttribSize);

		for (size_t i = 0; i < m_VertexArrays.size(); i++) 
		{
			auto vkVAO = dynamic_cast<VulkanVertexArray*>(m_VertexArrays[i].get());
			const auto& bindingDescs = vkVAO->GetBindingDescription();
			bindingDescriptions.insert(bindingDescriptions.end(), bindingDescs.begin(), bindingDescs.end());
			const auto& attribDescs = vkVAO->GetAttributeDescription();
			attributeDescriptions.insert(attributeDescriptions.end(), attribDescs.begin(), attribDescs.end());
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = Utils::DrawTypeToVk(p_CreateInfo.DrawType);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		std::vector<VkDynamicState> dynamicStateDescriptors = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateDescriptors.size();
		dynamicState.pDynamicStates = dynamicStateDescriptors.data();
			
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;
		viewportState.pViewports = nullptr;


		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = Utils::PolygonModeToVk(p_CreateInfo.PolygonMode);
		rasterizer.lineWidth = p_CreateInfo.LineWidth;
		rasterizer.cullMode = Utils::CullModeToVk(p_CreateInfo.CullMode);
		rasterizer.frontFace = Utils::FrontFaceToVk(p_CreateInfo.FrontFace);
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.lineWidth = p_CreateInfo.LineWidth; // TODO: Verify if support wide lines

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = 0x0f;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		if (p_CreateInfo.TransparencyEnabled)
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

			if (p_CreateInfo.BlendMode == BlendMode::SrcAlphaOneMinusSrcAlpha)
			{
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			}
			else if (p_CreateInfo.BlendMode == BlendMode::SrcAlphaOne)
			{
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			}
			else if (p_CreateInfo.BlendMode == BlendMode::ZeroSrcColor)
			{
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
			}
			else if (p_CreateInfo.BlendMode == BlendMode::OneZero)
			{
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			}
			else
			{
				colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			}
		}
		else
		{
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_NO_OP;
		colorBlending.blendConstants[0] = 1.0f;
		colorBlending.blendConstants[1] = 1.0f;
		colorBlending.blendConstants[2] = 1.0f;
		colorBlending.blendConstants[3] = 1.0f;


		auto shader = dynamic_cast<VulkanShader*>(m_Shader.get());
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = (uint32_t)shader->GetShaderStages().size();
		pipelineInfo.pStages = shader->GetShaderStages().data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = shader->GetLayout();
		pipelineInfo.renderPass = m_Context->GetRenderPass()->Get();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		auto res = vkCreateGraphicsPipelines(VulkanDevice::Get().GetDevice(), VulkanDevice::Get().GetPipelineCache(), 1, &pipelineInfo, VK_NULL_HANDLE, &m_Pipeline);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

	void VulkanPipeline::Invalidade()
	{
		CleanUp();

		Init(m_CreateInfo);
	}

	void VulkanPipeline::Invalidade(const PipelineCreateInfo& p_CreateInfo)
	{
		CleanUp();

		Init(p_CreateInfo);
	}

	void VulkanPipeline::Bind()
	{
		auto currentFrame = m_Context->GetCurrentFrame();
		auto commandBuffer = m_Context->GetCommandBuffer().Get(currentFrame);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}
}
