#include "YUME/yumepch.h"
#include "vulkan_shader.h"
#include <YUME/Utils/timer.h>
#include "YUME/Core/application.h"
#include "vulkan_context.h"
#include "Platform/Vulkan/Core/vulkan_device.h"

// Lib
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/resource_limits_c.h>
#include <spirv_cross/spirv_cross.hpp>
#include <glm/gtc/type_ptr.hpp>



const TBuiltInResource DefaultTBuiltInResource = {
	32,  // maxLights
	6,   // maxClipDistances
	8,   // maxCullDistances
	1,   // maxCombinedClipAndCullDistances
	2048, // maxCombinedShaderOutputResources
	4096, // maxComputeSharedMemorySize
	16,  // maxComputeWorkGroupCount
	1024, // maxComputeWorkGroupSize
	8,   // maxFragmentInputComponents
	64,  // maxImageUnits
	128, // maxImageSamples
	8,   // maxVertexOutputComponents
	8,   // maxTessControlOutputComponents
	16,  // maxTessEvaluationOutputComponents
	8,   // maxGeometryOutputComponents
	64,  // maxFragmentOutputAttachments
	8,   // maxGeometryInputComponents
	8,   // maxGeometryOutputComponents
	256, // maxFragmentCombinedOutputResources
	64,  // maxComputeWorkGroupInvocations
	16,  // maxWorkGroupSize
	1024, // maxWorkGroupCount
	8,   // maxGeometryOutputVertices
	1024, // maxGeometryTotalOutputComponents
	16,  // maxFragmentInputComponents
	16,  // maxVertexInputComponents
	1024, // maxTessControlInputComponents
	1024, // maxTessEvaluationInputComponents
	256, // maxTessControlOutputComponents
	1024, // maxTessEvaluationOutputComponents
	2048, // maxShaderStorageBufferBindings
	2048, // maxShaderStorageBufferSize
	128, // maxAtomicCounterBindings
	1024, // maxAtomicCounterBufferSize
	32,  // maxShaderImageSize
	2048, // maxShaderResourceSize
	64,  // maxShaderSamplerSize
	8,   // maxShaderConstantSize
	8,   // maxShaderPushConstantSize
	1024, // maxShaderUniformBufferSize
	128, // maxShaderStorageBufferSize
	1024, // maxShaderAtomicCounterSize
	256, // maxShaderAtomicCounterBindings
	2048, // maxShaderStorageBufferBindings
	256, // maxShaderStorageBufferSize
	2048, // maxShaderResourceSize
	128, // maxShaderSamplerSize
	128, // maxShaderSampledImageSize
	64,  // maxShaderImageSize
	8,   // maxShaderConstantSize
	16,  // maxShaderPushConstantSize
	256, // maxShaderUniformBufferSize
	2048, // maxShaderStorageBufferBindings
	256, // maxShaderStorageBufferSize
	2048, // maxShaderResourceSize
	128, // maxShaderSamplerSize
	128, // maxShaderSampledImageSize
	64,  // maxShaderImageSize
	8,   // maxShaderConstantSize
	16,  // maxShaderPushConstantSize
};


#define UPLOAD_PUSH_CONSTANT_DATA(sizeBytes, valuePtr)											\
	YM_CORE_VERIFY(m_PushConstants.find(p_Name) != m_PushConstants.end())						\
																								\
	auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());	\
	auto commandBuffer = context->GetCommandBuffer();											\
																								\
	vkCmdPushConstants(																			\
		commandBuffer,																			\
		m_PipelineLayout,																		\
		m_PushConstants[p_Name].stageFlags,														\
		m_PushConstants[p_Name].offset,															\
		sizeBytes,																				\
		valuePtr																				\
	);																						


namespace YUME
{
	namespace Utils
	{
		static ShaderType ShaderTypeFromString(const std::string_view& p_Type)
		{
			if (p_Type == "vertex")
				return ShaderType::VERTEX;
			if (p_Type == "fragment" || p_Type == "pixel")
				return ShaderType::FRAGMENT;

			YM_CORE_ASSERT(false, "Unknown shader type!")
			return (ShaderType)0;
		}

		static EShLanguage ShaderTypeToEShLang(ShaderType p_Type)
		{
			switch (p_Type)
			{
				case YUME::ShaderType::VERTEX: return EShLangVertex;
				case YUME::ShaderType::FRAGMENT: return EShLangFragment;
				default:
					YM_CORE_ASSERT(false, "Unknown shader type!")
					return (EShLanguage)0;
			}
		}

		static VkShaderStageFlagBits ShaderTypeToVK(ShaderType p_Type)
		{
			switch (p_Type)
			{
			case ShaderType::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
			case ShaderType::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
			default:
				YM_CORE_ERROR("Unknown shader type")
					return (VkShaderStageFlagBits)0;
			}
		}

		static std::string_view ShaderStageToString(ShaderType p_Stage)
		{
			switch (p_Stage)
			{
				case ShaderType::VERTEX:   return "VERTEX";
				case ShaderType::FRAGMENT: return "FRAGMENT";
				default:
					YM_CORE_ERROR("Unknown shader stage")
					return "Unknown";
			}
		}

		static void CreateDirectoryIfNeeded(const std::string_view& p_Path)
		{
			if (!std::filesystem::exists(p_Path))
				std::filesystem::create_directories(p_Path);
		}

		static const char* ShaderStageCachedVulkanFileExtension(ShaderType p_Stage)
		{
			switch (p_Stage)
			{
				case ShaderType::VERTEX:    return ".cached_vulkan.vert";
				case ShaderType::FRAGMENT:  return ".cached_vulkan.frag";
				default:
					YM_CORE_ASSERT(false)
					return "";
			}
		}
	}

	static const std::filesystem::path s_CacheDirectory = "assets/cache/shaders";

	VulkanShader::VulkanShader(const std::string_view& p_ShaderPath)
		: m_FilePath(p_ShaderPath)
	{
		Utils::CreateDirectoryIfNeeded(s_CacheDirectory.string());

		std::string source = ReadFile(p_ShaderPath);
		auto shaderSources = PreProcess(source);


		Timer timer;
		timer.Start();
	
		CompileOrGetVulkanBinaries(shaderSources);
		CreateShaderModules();
		CreatePipelineLayout();

		timer.Stop();
		YM_CORE_WARN("Shader creation took {0} ms", timer.Elapsed())

		// Extract name from shaderPath
		auto lastSlash = p_ShaderPath.find_last_of("/\\");
		lastSlash = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
		auto lastDot = p_ShaderPath.rfind('.');
		lastDot = (lastDot == std::string::npos) ? p_ShaderPath.size() : lastDot;

		m_Name = p_ShaderPath.substr(lastSlash, lastDot - lastSlash);
	}

	VulkanShader::~VulkanShader()
	{
		auto shaderModules = m_ShaderModules;
		auto layout = m_PipelineLayout;
		VulkanContext::PushFunction([shaderModules, layout]()
		{
			auto device = VulkanDevice::Get().GetDevice();

			for (const auto& [stage, shaderModule] : shaderModules)
			{
				YM_CORE_TRACE("Destroying vulkan shader modules...")
				if (shaderModule != VK_NULL_HANDLE)
					vkDestroyShaderModule(device, shaderModule, VK_NULL_HANDLE);
			}

			YM_CORE_TRACE("Destroying vulkan pipeline layout...")
			vkDestroyPipelineLayout(device, layout, VK_NULL_HANDLE);
		});
	};

	void VulkanShader::CleanUp()
	{
		auto device = VulkanDevice::Get().GetDevice();

		for (const auto& [stage, shaderModule] : m_ShaderModules)
		{
			YM_CORE_TRACE("Destroying vulkan shader modules...")
			if (shaderModule != VK_NULL_HANDLE)
				vkDestroyShaderModule(device, shaderModule, VK_NULL_HANDLE);
		}

		YM_CORE_TRACE("Destroying vulkan pipeline layout...")
		vkDestroyPipelineLayout(device, m_PipelineLayout, VK_NULL_HANDLE);
	}

	void VulkanShader::Bind()
	{
		if (auto pipeline = m_Pipeline.lock()) 
		{
			pipeline->Bind();
		}
		else 
		{
			YM_CORE_ERROR("Pipelie has deleted!")
		}
	}

	void VulkanShader::Unbind()
	{
		// Nothing
	}

	const std::string_view& VulkanShader::GetName() const
	{
		return m_Name;
	}

	void VulkanShader::UploadFloat(const std::string& p_Name, float p_Value)
	{
		UPLOAD_PUSH_CONSTANT_DATA(sizeof(float), &p_Value)
	}

	void VulkanShader::UploadFloat3(const std::string& p_Name, const glm::vec3& p_Value)
	{
		UPLOAD_PUSH_CONSTANT_DATA(sizeof(glm::vec3), glm::value_ptr(p_Value))
	}

	void VulkanShader::UploadFloat4(const std::string& p_Name, const glm::vec4& p_Value)
	{
		UPLOAD_PUSH_CONSTANT_DATA(sizeof(glm::vec4), glm::value_ptr(p_Value))
	}

	void VulkanShader::UploadInt(const std::string& p_Name, int p_Value)
	{
		UPLOAD_PUSH_CONSTANT_DATA(sizeof(int), &p_Value)
	}

	std::string VulkanShader::ReadFile(const std::string_view& p_Filepath) const
	{
		std::string result;
		std::ifstream shaderFile{ p_Filepath.data(), std::ios::in | std::ios::binary};

		if (!shaderFile)
		{
			YM_CORE_ERROR("Could not open file '{0}' ", p_Filepath)
			return std::string();
		}

		shaderFile.seekg(0, std::ios::end);
		result.resize(shaderFile.tellg());
		shaderFile.seekg(0, std::ios::beg);

		shaderFile.read(&result[0], result.size());
		shaderFile.close();

		return result;
	}

	VulkanShader::ShaderSource VulkanShader::PreProcess(const std::string& p_Source) const
	{
		ShaderSource shaderSources;

		const char* typeToken = "@type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = p_Source.find(typeToken, 0);
		while (pos != std::string::npos)
		{
			size_t eol = p_Source.find_first_of("\r\n", pos);
			YM_CORE_VERIFY(eol != std::string::npos, "Syntax error")

			size_t begin = p_Source.find_first_not_of(" \t", pos + typeTokenLength);
			size_t end = p_Source.find_last_not_of(" \t", eol);
			std::string type = p_Source.substr(begin, end - begin + 1);
			if (size_t typeEnd = type.find_first_of(" \r\n");
				typeEnd != std::string::npos) {
				type.erase(typeEnd);
			}

			// Encontrar a pr�xima linha de shader
			size_t nextLinePos = p_Source.find_first_not_of("\r\n", eol);
			pos = p_Source.find(typeToken, nextLinePos);
			shaderSources[Utils::ShaderTypeFromString(type)] = p_Source.substr(
				nextLinePos,
				pos - (nextLinePos == std::string::npos ? p_Source.size() - 1 : nextLinePos)
			);
		}

		return shaderSources;
	}

	void VulkanShader::CompileOrGetVulkanBinaries(const ShaderSource& p_ShaderSources)
	{
		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : p_ShaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = s_CacheDirectory / (shaderFilePath.filename().string() + Utils::ShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				glslang::InitializeProcess();

				auto shaderType = Utils::ShaderTypeToEShLang(stage);
				glslang::TShader shader(shaderType);
				const char* shaderStrings = source.c_str();
				shader.setStrings(&shaderStrings, 1);

				int ClientInputSemanticsVersion = 100;
				glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_2;
				glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;

				shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, ClientInputSemanticsVersion);
				shader.setEnvClient(glslang::EShClientVulkan, VulkanClientVersion);
				shader.setEnvTarget(glslang::EShTargetSpv, TargetVersion);

				TBuiltInResource Resources = DefaultTBuiltInResource;
				
				if (auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
					!shader.parse(&Resources, 100, false, messages))
				{
					YM_CORE_ERROR("GLSL Parsing Failed for shader: ")
					YM_CORE_ERROR("{}", shader.getInfoLog())
					YM_CORE_ERROR("{}", shader.getInfoDebugLog())
					YM_CORE_ASSERT(false)
				}

				glslang::TProgram program;
				program.addShader(&shader);

				if (auto messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
					!program.link(messages))
				{
					YM_CORE_ERROR("Program Linking Failed")
					YM_CORE_ERROR("{}", program.getInfoLog())
					YM_CORE_ERROR("{}", program.getInfoDebugLog())
					YM_CORE_ASSERT(false)
				}

				std::vector<uint32_t> spirv;
				glslang::GlslangToSpv(*program.getIntermediate(shaderType), spirv);

				shaderData[stage] = spirv;
				
				if (std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
					out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}

				glslang::FinalizeProcess();
			}
		}

		for (const auto& [stage, data] : shaderData)
		{
			Reflect(stage, data);
		}

	}

	void VulkanShader::Reflect(ShaderType p_Stage, const std::vector<uint32_t>& p_ShaderData)
	{
		spirv_cross::Compiler compiler(p_ShaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		YM_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::ShaderStageToString(p_Stage), m_FilePath)
		YM_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size())
		YM_CORE_TRACE("    {0} resources", resources.sampled_images.size())

		YM_CORE_TRACE("Uniform buffers:")
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = (int)bufferType.member_types.size();

			YM_CORE_TRACE("  Name: {0}", resource.name)
			YM_CORE_TRACE("    Size = {0}", bufferSize)
			YM_CORE_TRACE("    Binding = {0}", binding)
			YM_CORE_TRACE("    Members = {0}", memberCount)
		}

		YM_CORE_TRACE("Push constant buffers:")
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = (int)bufferType.member_types.size();

			YM_CORE_TRACE("  Name: {0}", resource.name)
			YM_CORE_TRACE("    Size = {0}", bufferSize)
			YM_CORE_TRACE("    Binding = {0}", binding)
			YM_CORE_TRACE("    Members = {0}", memberCount)

			const auto& ranges = compiler.get_active_buffer_ranges(resource.id);

			for (const auto& range : ranges) 
			{
				PushConstantRangeInfo info;
				info.Name = resource.name;
				info.Offset = m_CurrentOffset;
				info.Size = range.range;
				info.Stages = Utils::ShaderTypeToVK(p_Stage);
				m_RangeInfos.push_back(info);

				m_CurrentOffset += range.range;
			}
		}
	}

	void VulkanShader::CreateShaderModules()
	{
		auto device = VulkanDevice::Get().GetDevice();

		for (auto& [stage, code] : m_VulkanSPIRV)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size() * sizeof(uint32_t);
			createInfo.pCode = code.data();
			
			auto res = vkCreateShaderModule(device, &createInfo, nullptr, &m_ShaderModules[stage]);
			YM_CORE_ASSERT(res == VK_SUCCESS)

			VkPipelineShaderStageCreateInfo ShaderStageInfo{};
			ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			ShaderStageInfo.stage = Utils::ShaderTypeToVK(stage);
			ShaderStageInfo.module = m_ShaderModules[stage];
			ShaderStageInfo.pName = "main";
			m_ShaderStages.push_back(ShaderStageInfo);
		}

	}

	void VulkanShader::CreatePipelineLayout()
	{
		for (const auto& rangeInfo : m_RangeInfos) 
		{
			bool merged = false;
			for (auto& vkRange : m_PushConstantRanges)
			{
				if (vkRange.offset == rangeInfo.Offset && vkRange.size == rangeInfo.Size)
				{
					vkRange.stageFlags |= rangeInfo.Stages;
					merged = true;
					break;
				}
			}

			if (!merged)
			{
				VkPushConstantRange vkRange = {};
				vkRange.offset = (uint32_t)rangeInfo.Offset;
				vkRange.size = (uint32_t)rangeInfo.Size;

				VkShaderStageFlags flags = 0;
				for (const auto& shaderStages : m_ShaderStages)
				{
					flags |= shaderStages.stage;
				}

				vkRange.stageFlags = flags;
				m_PushConstantRanges.push_back(vkRange);

				m_PushConstants[rangeInfo.Name] = vkRange;
			}
		}


		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = (uint32_t)m_PushConstantRanges.size();
		layoutInfo.pPushConstantRanges = m_PushConstantRanges.data();

		auto device = VulkanDevice::Get().GetDevice();

		auto  res = vkCreatePipelineLayout(device, &layoutInfo, VK_NULL_HANDLE, &m_PipelineLayout);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}
}