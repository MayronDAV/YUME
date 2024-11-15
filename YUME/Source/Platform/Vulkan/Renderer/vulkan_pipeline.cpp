#include "YUME/yumepch.h"
#include "vulkan_pipeline.h"
#include "Platform/Vulkan/Core/vulkan_device.h"
#include "vulkan_context.h"
#include "vulkan_shader.h"
#include "YUME/Renderer/renderer_command.h"
#include "YUME/Core/application.h"
#include "vulkan_texture.h"
#include <glm/gtc/type_ptr.hpp>




namespace YUME
{
	VulkanPipeline::VulkanPipeline(const PipelineCreateInfo& p_CreateInfo)
	{
		Init(p_CreateInfo);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying vertex array...")

		auto pipeline = m_Pipeline;
		VulkanContext::PushFunction([pipeline]()
		{
			YM_CORE_TRACE(VULKAN_PREFIX "Destroying pipeline...")
			if (pipeline != VK_NULL_HANDLE)
				vkDestroyPipeline(VulkanDevice::Get().GetDevice(), pipeline, VK_NULL_HANDLE);
		});
	}

	void VulkanPipeline::CleanUp()
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_TRACE(VULKAN_PREFIX "Destroying pipeline...")
		if (m_Pipeline != VK_NULL_HANDLE)
			vkDestroyPipeline(VulkanDevice::Get().GetDevice(), m_Pipeline, VK_NULL_HANDLE);
	}

	void VulkanPipeline::Init(const PipelineCreateInfo& p_CreateInfo)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_ASSERT(p_CreateInfo.Shader != nullptr)

		m_Context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		m_CreateInfo = p_CreateInfo;
		m_Shader = p_CreateInfo.Shader;

		TransitionAttachments();
		CreateFramebuffers();

		auto shader					= m_Shader.As<VulkanShader>();
		size_t totalBindingSize		= shader->GetBindingDescription().size();
		size_t totalInputAttribSize = shader->GetAttributeDescription().size();

		const auto& bindingDescs	= shader->GetBindingDescription();
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(bindingDescs.begin(), bindingDescs.end(), bindingDescs.get_allocator());
		const auto& attribDescs		= shader->GetAttributeDescription();
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(attribDescs.begin(), attribDescs.end(), attribDescs.get_allocator());

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount	= (uint32_t)bindingDescriptions.size();
		vertexInputInfo.pVertexBindingDescriptions		= bindingDescriptions.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();
		
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType					 = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology				 = VKUtils::DrawTypeToVk(p_CreateInfo.DrawType);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		std::vector<VkDynamicState> dynamicStateDescriptors = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};


		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount  = (uint32_t)dynamicStateDescriptors.size();
		dynamicState.pDynamicStates		= dynamicStateDescriptors.data();
			
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType				= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount		= 1;
		viewportState.scissorCount		= 1;
		viewportState.pScissors			= nullptr;
		viewportState.pViewports		= nullptr;


		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable			= VK_FALSE;
		rasterizer.rasterizerDiscardEnable  = VK_FALSE;
		if (p_CreateInfo.PolygonMode == PolygonMode::LINE || p_CreateInfo.PolygonMode == PolygonMode::POINT)
		{
			rasterizer.polygonMode = VKUtils::PolygonModeToVk((RendererCommand::GetCapabilities().FillModeNonSolid) ? p_CreateInfo.PolygonMode : PolygonMode::FILL);
		}
		else
		{
			rasterizer.polygonMode = VKUtils::PolygonModeToVk(p_CreateInfo.PolygonMode);
		}
		rasterizer.lineWidth		= p_CreateInfo.LineWidth;
		rasterizer.cullMode			= VKUtils::CullModeToVk(p_CreateInfo.CullMode);
		rasterizer.frontFace		= VKUtils::FrontFaceToVk(p_CreateInfo.FrontFace);
		rasterizer.depthBiasEnable  = VK_FALSE;
		if (RendererCommand::GetCapabilities().WideLines)
			rasterizer.lineWidth	= p_CreateInfo.LineWidth;
		else
			rasterizer.lineWidth	= 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable   = VK_FALSE;
		multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkPipelineColorBlendAttachmentState> blendAttachState;
		blendAttachState.resize(m_RenderPass.As<VulkanRenderPass>()->GetColorAttachmentCount());
		for (size_t i = 0; i < blendAttachState.size(); i++)
		{
			blendAttachState[i].colorWriteMask		= 0x0f;
			blendAttachState[i].alphaBlendOp		= VK_BLEND_OP_ADD;
			blendAttachState[i].colorBlendOp		= VK_BLEND_OP_ADD;
			blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

			if (p_CreateInfo.TransparencyEnabled)
			{
				blendAttachState[i].blendEnable = VK_TRUE;
				blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;

				if (p_CreateInfo.BlendModes[i] == BlendMode::SrcAlphaOneMinusSrcAlpha)
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				}
				else if (p_CreateInfo.BlendModes[i] == BlendMode::SrcAlphaOne)
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
					blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				}
				else if (p_CreateInfo.BlendModes[i] == BlendMode::ZeroSrcColor)
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
				}
				else if (p_CreateInfo.BlendModes[i] == BlendMode::OneZero)
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				}
				else if (p_CreateInfo.BlendModes[i] == BlendMode::OneOne)
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				}
				else if (p_CreateInfo.BlendModes[i] == BlendMode::ZeroOneMinusSrcColor)
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
				}
				else
				{
					blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				}
			}
			else
			{
				blendAttachState[i].blendEnable			= VK_FALSE;
				blendAttachState[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachState[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachState[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				blendAttachState[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			}
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount	= (uint32_t)blendAttachState.size();
		colorBlending.pAttachments		= blendAttachState.data();
		colorBlending.logicOpEnable		= VK_FALSE;
		colorBlending.logicOp			= VK_LOGIC_OP_NO_OP;
		colorBlending.blendConstants[0] = 1.0f;
		colorBlending.blendConstants[1] = 1.0f;
		colorBlending.blendConstants[2] = 1.0f;
		colorBlending.blendConstants[3] = 1.0f;

		
		VkPipelineDepthStencilStateCreateInfo ds{};
		ds.sType				 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext				 = NULL;
		ds.depthTestEnable		 = p_CreateInfo.DepthTest ? VK_TRUE : VK_FALSE;
		ds.depthWriteEnable		 = p_CreateInfo.DepthWrite ? VK_TRUE : VK_FALSE;
		ds.depthCompareOp		 = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.stencilTestEnable	 = VK_FALSE;
		ds.back.failOp			 = VK_STENCIL_OP_KEEP;
		ds.back.passOp			 = VK_STENCIL_OP_KEEP;
		ds.back.compareOp		 = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask		 = 0;
		ds.back.reference		 = 0;
		ds.back.depthFailOp		 = VK_STENCIL_OP_KEEP;
		ds.back.writeMask		 = 0;
		ds.minDepthBounds		 = 0;
		ds.maxDepthBounds		 = 0;
		ds.front				 = ds.back;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType				 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount			 = (uint32_t)shader->GetShaderStages().size();
		pipelineInfo.pStages			 = shader->GetShaderStages().data();
		pipelineInfo.pVertexInputState	 = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState		 = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState   = &multisampling;
		pipelineInfo.pDepthStencilState  = &ds;
		pipelineInfo.pColorBlendState	 = &colorBlending;
		pipelineInfo.pDynamicState		 = &dynamicState;
		pipelineInfo.layout				 = shader->GetLayout();
		pipelineInfo.renderPass			 = m_RenderPass.As<VulkanRenderPass>()->Get();
		pipelineInfo.subpass			 = 0;
		pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex   = -1; // Optional

		auto res = vkCreateGraphicsPipelines(VulkanDevice::Get().GetDevice(), VulkanDevice::Get().GetPipelineCache(), 1, &pipelineInfo, VK_NULL_HANDLE, &m_Pipeline);
		YM_CORE_VERIFY(res == VK_SUCCESS)

		if (!m_CreateInfo.DebugName.empty())
			VKUtils::SetDebugUtilsObjectName(VulkanDevice::Get().GetDevice(), VK_OBJECT_TYPE_PIPELINE, m_CreateInfo.DebugName.c_str(), m_Pipeline);
	}

	void VulkanPipeline::Invalidade()
	{
		YM_PROFILE_FUNCTION()

		CleanUp();

		Init(m_CreateInfo);
	}

	void VulkanPipeline::Invalidade(const PipelineCreateInfo& p_CreateInfo)
	{
		YM_PROFILE_FUNCTION()

		CleanUp();

		Init(p_CreateInfo);
	}

	void VulkanPipeline::Begin(CommandBuffer* p_CommandBuffer, SubpassContents p_Contents)
	{
		YM_PROFILE_FUNCTION()

		if (m_CreateInfo.SwapchainTarget)
		{
			ResizeFramebuffer();
		}

		TransitionAttachments();

		auto commandBuffer = static_cast<VulkanCommandBuffer*>(p_CommandBuffer)->GetHandle();

		m_RenderPass->Begin(p_CommandBuffer, GetFramebuffer(), GetWidth(), GetHeight(), glm::make_vec4(m_CreateInfo.ClearColor), p_Contents);

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}

	void VulkanPipeline::End(CommandBuffer* p_CommandBuffer)
	{
		YM_PROFILE_FUNCTION()

		m_RenderPass->End(p_CommandBuffer);
	}

	const Ref<Framebuffer>& VulkanPipeline::GetFramebuffer() const
	{
		if (m_CreateInfo.SwapchainTarget)
		{
			return m_Framebuffers[VulkanSwapchain::Get().GetCurrentBuffer()];
		}

		return m_Framebuffers[0];
	}

	void VulkanPipeline::TransitionAttachments()
	{
		YM_PROFILE_FUNCTION()

		auto cmd = VulkanSwapchain::Get().GetCurrentFrameData().MainCommandBuffer.get();

		for (const auto& texture : m_CreateInfo.ColorTargets)
		{
			if (texture != nullptr)
			{
				texture.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, cmd);
			}
		}

		if (m_CreateInfo.DepthTarget)
		{
			m_CreateInfo.DepthTarget.As<VulkanTexture2D>()->TransitionImage(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, cmd);
		}
	}

	void VulkanPipeline::CreateFramebuffers()
	{
		YM_PROFILE_FUNCTION()

		std::vector<Ref<Texture2D>> attachments;
		auto width = GetWidth();
		auto height = GetHeight();

		if (m_CreateInfo.SwapchainTarget)
		{
			TextureSpecification txSpec{};
			txSpec.Width			= width;
			txSpec.Height			= height;
			txSpec.Format			= TextureFormat::RGBA8_SRGB;
			txSpec.Usage			= TextureUsage::TEXTURE_COLOR_ATTACHMENT;
			txSpec.GenerateMips		= false;
			txSpec.AnisotropyEnable = false;
			txSpec.DebugName		= "SwapchainFramebufferAttachment";

			attachments.push_back(Texture2D::Get(txSpec));
		}
		else
		{
			for (const auto& texture : m_CreateInfo.ColorTargets)
			{
				if (texture != nullptr)
				{
					attachments.push_back(texture);
				}
			}
		}

		if (m_CreateInfo.DepthTarget)
		{
			attachments.push_back(m_CreateInfo.DepthTarget);
		}

		RenderPassSpecification spec{};
		spec.Attachments	 = attachments;
		spec.ClearEnable	 = m_CreateInfo.ClearTargets;
		spec.SwapchainTarget = m_CreateInfo.SwapchainTarget;
		spec.DebugName		 = m_CreateInfo.DebugName + "- RenderPass";
		m_RenderPass		 = RenderPass::Get(spec);

		FramebufferSpecification fbSpec{};
		fbSpec.RenderPass	 = m_RenderPass;
		fbSpec.Width		 = width;
		fbSpec.Height		 = height;
		fbSpec.DebugName	 = m_CreateInfo.DebugName + "- Framebuffer";

		if (m_CreateInfo.SwapchainTarget)
		{
			const auto& buffers	   = VulkanSwapchain::Get().GetBuffers();
			uint32_t size		   = VulkanSwapchain::Get().GetBufferCount();


			for (uint32_t i = 0; i < size; i++)
			{
				attachments[0]	   = buffers[i];
				fbSpec.Attachments = attachments;

				m_Framebuffers.push_back(Framebuffer::Get(fbSpec));
			}
		}
		else
		{
			fbSpec.Attachments	   = attachments;
			m_Framebuffers.push_back(Framebuffer::Get(fbSpec));
		}
	}

	void VulkanPipeline::ResizeFramebuffer()
	{
		const auto& buffers = VulkanSwapchain::Get().GetBuffers();
		auto currentFrame   = VulkanSwapchain::Get().GetCurrentBuffer();

		auto width			= buffers[currentFrame]->GetWidth();
		auto height			= buffers[currentFrame]->GetHeight();

		if (m_Framebuffers[currentFrame]->GetWidth()  != width ||
			m_Framebuffers[currentFrame]->GetHeight() != height)
		{
			m_Framebuffers[currentFrame]->OnResize(width, height);
		}
	}
}
