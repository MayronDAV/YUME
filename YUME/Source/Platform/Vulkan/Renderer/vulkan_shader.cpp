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
#include "vulkan_uniform_buffer.h"
#include "vulkan_texture.h"



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
		YM_PROFILE_FUNCTION()

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
		YM_PROFILE_FUNCTION()

		auto shaderModules = m_ShaderModules;
		auto layout = m_PipelineLayout;
		auto descriptorSetLayouts = m_DescriptorSetLayouts;
		VulkanContext::PushFunction([shaderModules, layout, descriptorSetLayouts]()
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

			YM_CORE_TRACE("Destroying vulkan descriptor set layout...")
			for (const auto& [set, setLayout] : descriptorSetLayouts)
			{
				vkDestroyDescriptorSetLayout(device, setLayout, VK_NULL_HANDLE);
			}
		});
	};

	void VulkanShader::CleanUp()
	{
		YM_PROFILE_FUNCTION()

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
		YM_PROFILE_FUNCTION()

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
	}

	const std::string_view& VulkanShader::GetName() const
	{
		return m_Name;
	}

	void VulkanShader::PushFloat(const std::string& p_Name, float p_Value)
	{
		UploadPushConstantData(p_Name, &p_Value, sizeof(float));
	}

	void VulkanShader::PushFloat3(const std::string& p_Name, const glm::vec3& p_Value)
	{
		UploadPushConstantData(p_Name, glm::value_ptr(p_Value), sizeof(glm::vec3));
	}

	void VulkanShader::PushFloat4(const std::string& p_Name, const glm::vec4& p_Value)
	{
		UploadPushConstantData(p_Name, glm::value_ptr(p_Value), sizeof(glm::vec4));
	}

	void VulkanShader::PushMat4(const std::string& p_Name, const glm::mat4& p_Value)
	{
		UploadPushConstantData(p_Name, glm::value_ptr(p_Value), sizeof(glm::mat4));
	}

	void VulkanShader::PushInt(const std::string& p_Name, int p_Value)
	{
		UploadPushConstantData(p_Name, &p_Value, sizeof(int));
	}


	std::string VulkanShader::ReadFile(const std::string_view& p_Filepath) const
	{
		YM_PROFILE_FUNCTION()

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

	std::string VulkanShader::ProcessIncludeFiles(const std::string& p_Code) const
	{
		YM_PROFILE_FUNCTION()

		auto result = std::string();

		const char* includeToken = "#include";
		size_t pos = p_Code.find(includeToken, 0);
		if (pos == std::string::npos)
		{
			YM_CORE_TRACE("Not founded any include! continuing...")
			return p_Code;
		}

		while (pos != std::string::npos)
		{
			if (result == std::string())
			{
				result = p_Code.substr(0, pos);
			}
			
			size_t eol = p_Code.find_first_of("\r\n", pos);
			size_t start = p_Code.find("<", pos);
			size_t end = p_Code.find(">", start);

			YM_CORE_VERIFY(start != std::string::npos && end != std::string::npos, "Invalid include directive!")
			YM_CORE_VERIFY(start < eol && end < eol, "Not on the same line!")

			std::string includeFilepath = p_Code.substr(start + 1, end - start - 1);
			std::filesystem::path filepath = m_FilePath;
			auto path = (filepath.parent_path() / includeFilepath).string();
			std::string includeCode = ReadFile(path);

			result += includeCode;

			pos = p_Code.find(includeToken, end + 1);
			if (pos == std::string::npos)
			{
				result += p_Code.substr(end + 1);
			}
			else
			{
				result += p_Code.substr(end + 1, pos - end - 1);
			}
		}

		return result;
	}


	VulkanShader::ShaderSource VulkanShader::PreProcess(const std::string& p_Source) const
	{
		YM_PROFILE_FUNCTION()

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

			size_t nextLinePos = p_Source.find_first_not_of("\r\n", eol);
			pos = p_Source.find(typeToken, nextLinePos);
			auto code = p_Source.substr(
				nextLinePos,
				pos - (nextLinePos == std::string::npos ? p_Source.size() - 1 : nextLinePos)
			);
			auto newCode = ProcessIncludeFiles(code);
			shaderSources[Utils::ShaderTypeFromString(type)] = newCode;
		}

		return shaderSources;
	}

	void VulkanShader::CompileOrGetVulkanBinaries(const ShaderSource& p_ShaderSources)
	{
		YM_PROFILE_FUNCTION()

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
		YM_PROFILE_FUNCTION()

		spirv_cross::Compiler compiler(p_ShaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		YM_CORE_TRACE("Vulkan::Reflect - {0} {1}", Utils::ShaderStageToString(p_Stage), m_FilePath)
		YM_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size())
		YM_CORE_TRACE("    {0} resources", resources.sampled_images.size())

		YM_CORE_TRACE("Uniform buffers:")
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = (int)bufferType.member_types.size();
			const auto& Type = compiler.get_type(resource.type_id);
			auto descriptorCount = (Type.array.size() > 0) ? Type.array[0] : 1;

			YM_CORE_TRACE("  Name: {0}", resource.name)
			YM_CORE_TRACE("    Size = {0}", bufferSize)
			YM_CORE_TRACE("    Set  = {0}", set)
			YM_CORE_TRACE("    Binding = {0}", binding)
			YM_CORE_TRACE("    Members = {0}", memberCount)
			YM_CORE_TRACE("    Descriptor Count = {0}", descriptorCount)

			VkDescriptorSetLayoutBinding bindingInfo{};
			bindingInfo.binding = binding;
			bindingInfo.descriptorCount = descriptorCount;
			bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindingInfo.stageFlags = Utils::ShaderTypeToVK(p_Stage);
			bindingInfo.pImmutableSamplers = nullptr;

			m_DescriptorSetLayoutBindings[set].push_back(bindingInfo);
		}

		YM_CORE_TRACE("Sampled images:")
		for (const auto& resource : resources.sampled_images)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = (int)bufferType.member_types.size();
			const auto& Type = compiler.get_type(resource.type_id);
			auto descriptorCount = (Type.array.size() > 0) ? Type.array[0] : 1;

			YM_CORE_TRACE("  Name: {0}", resource.name)
			YM_CORE_TRACE("    Set  = {0}", set)
			YM_CORE_TRACE("    Binding = {0}", binding)
			YM_CORE_TRACE("    Members = {0}", memberCount)
			YM_CORE_TRACE("    Descriptor Count = {0}", descriptorCount)

			VkDescriptorSetLayoutBinding bindingInfo{};
			bindingInfo.binding = binding;
			bindingInfo.descriptorCount = descriptorCount;
			bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindingInfo.stageFlags = Utils::ShaderTypeToVK(p_Stage);
			bindingInfo.pImmutableSamplers = nullptr;

			m_DescriptorSetLayoutBindings[set].push_back(bindingInfo);
		}

		YM_CORE_TRACE("Push constant buffers:")
		for (const auto& resource : resources.push_constant_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto size = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t offset = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = (int)bufferType.member_types.size();
			std::string resourceName = resource.name;

			YM_CORE_TRACE("  Name: {0}", resourceName)
			YM_CORE_TRACE("    Size = {0}", size)
			YM_CORE_TRACE("    Offset = {0}", offset)
			YM_CORE_TRACE("    Members = {0}", memberCount)


			auto stage = Utils::ShaderTypeToVK(p_Stage);
			if (m_PushConstantRanges.contains(offset))
			{
				m_PushConstantRanges[offset].stageFlags |= stage;
				m_PushConstantRanges[offset].size = std::max(m_PushConstantRanges[offset].size, size);

				for (int i = 0; i < memberCount; i++)
				{
					std::string name = compiler.get_member_name(resource.base_type_id, i);
					std::string memberName = resourceName.empty() ? name : resourceName + "." + name;
					m_MemberOffsets[memberName].first |= stage;
				}
			}
			else
			{
				m_PushConstantRanges[offset].stageFlags = stage;
				m_PushConstantRanges[offset].offset = offset;
				m_PushConstantRanges[offset].size = size;

				for (int i = 0; i < memberCount; i++)
				{
					std::string name = compiler.get_member_name(resource.base_type_id, i);
					std::string memberName = resourceName.empty() ? name : resourceName + "." + name;
					uint32_t memberOffset = compiler.type_struct_member_offset(bufferType, i);
					m_MemberOffsets[memberName] = std::make_pair(stage, offset + memberOffset);
				}
			}
		}
	}

	void VulkanShader::CreateShaderModules()
	{
		YM_PROFILE_FUNCTION()

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

			m_Stages |= Utils::ShaderTypeToVK(stage);
		}

	}

	void VulkanShader::CreatePipelineLayout()
	{
		YM_PROFILE_FUNCTION()

		auto device = VulkanDevice::Get().GetDevice();

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		for (const auto& [set, layoutBindings] : m_DescriptorSetLayoutBindings)
		{
			VkDescriptorSetLayoutCreateInfo SetLayoutInfo{};
			SetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			SetLayoutInfo.bindingCount = (uint32_t)layoutBindings.size();
			SetLayoutInfo.pBindings = layoutBindings.data();

			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(device, &SetLayoutInfo, VK_NULL_HANDLE, &layout);
			descriptorSetLayouts.push_back(layout);
			m_DescriptorSetLayouts[set] = layout;
		}


		std::vector<VkPushConstantRange> pushConstantRangeArray;
		for (const auto& range : m_PushConstantRanges) {
			pushConstantRangeArray.push_back(range.second);
		}

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		layoutInfo.pSetLayouts = descriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = (uint32_t)pushConstantRangeArray.size();
		layoutInfo.pPushConstantRanges = pushConstantRangeArray.data();	

		auto res = vkCreatePipelineLayout(device, &layoutInfo, VK_NULL_HANDLE, &m_PipelineLayout);
		YM_CORE_VERIFY(res == VK_SUCCESS)
	}

	bool VulkanShader::UploadPushConstantData(const std::string& p_Name, const void* p_Data, size_t p_SizeBytes)
	{
		YM_PROFILE_FUNCTION()

		YM_CORE_VERIFY(p_Data != nullptr && p_SizeBytes > 0)

		auto context = static_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		auto commandBuffer = context->GetCommandBuffer();
		
		if (auto it = m_MemberOffsets.find(p_Name);
			it != m_MemberOffsets.end())
		{
			auto [stage, offset] = it->second;
			vkCmdPushConstants(
				commandBuffer,
				m_PipelineLayout,
				m_Stages,
				offset,
				(uint32_t)p_SizeBytes,
				p_Data
			);

			return true;
		}

		return false;
	}
}