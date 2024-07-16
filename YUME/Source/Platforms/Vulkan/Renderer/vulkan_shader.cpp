#include "YUME/yumepch.h"
#include "vulkan_shader.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/resource_limits_c.h>
#include <spirv_cross/spirv_cross.hpp>
#include <YUME/Utils/timer.h>

#include "YUME/Core/application.h"
#include "vulkan_context.h"


const TBuiltInResource DefaultTBuiltInResource = {
	// initialization omitted for brevity
	// You can find the full initialization in the `StandAlone` tool of glslang
};



namespace YUME
{
	namespace Utils
	{
		static uint32_t ShaderTypeFromString(const std::string_view& p_Type)
		{
			if (p_Type == "vertex")
				return EShLangVertex;
			if (p_Type == "fragment" || p_Type == "pixel")
				return EShLangFragment;

			YM_CORE_ASSERT(false, "Unknown shader type!")
			return -1;
		}

		static uint32_t ShaderTypeForVKShaderType(uint32_t p_Type)
		{
			switch (p_Type)
			{
				case EShLangVertex: return VK_SHADER_STAGE_VERTEX_BIT;
				case EShLangFragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
				default:
					YM_CORE_ERROR("Unknown shader type")
					return 0;
			}
		}

		static std::string_view ShaderStageToString(uint32_t p_Stage)
		{
			switch (p_Stage)
			{
				case EShLangVertex:   return "EShLangVertex";
				case EShLangFragment: return "EShLangFragment";
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

		static const char* ShaderStageCachedVulkanFileExtension(uint32_t p_Stage)
		{
			switch (p_Stage)
			{
				case EShLangVertex:    return ".cached_vulkan.vert";
				case EShLangFragment:  return ".cached_vulkan.frag";
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
		static auto s_Context = dynamic_cast<VulkanContext*>(Application::Get().GetWindow().GetContext());
		m_Device = s_Context->GetDevice();

		Utils::CreateDirectoryIfNeeded(s_CacheDirectory.string());

		std::string source = ReadFile(p_ShaderPath);
		auto shaderSources = PreProcess(source);


		Timer timer;
		timer.Start();
	
		CompileOrGetVulkanBinaries(shaderSources);
		CreateShaderModules();

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
		YM_CORE_TRACE("Destroying vulkan shader modules")
		for (const auto& [stage, shaderModule] : m_ShaderModules)
		{
			vkDestroyShaderModule(m_Device, shaderModule, VK_NULL_HANDLE);
		}
	}

	void VulkanShader::Bind()
	{
	}

	void VulkanShader::Unbind()
	{
	}

	const std::string_view& VulkanShader::GetName() const
	{
		return m_Name;
	}

	void VulkanShader::UploadFloat(const std::string_view& p_Name, float p_Value)
	{
	}

	void VulkanShader::UploadInt(const std::string_view& p_Name, int p_Value)
	{
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
			YM_CORE_VERIFY(Utils::ShaderTypeFromString(type) != -1, "Invalid shader type specification")

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

				glslang::TShader shader((EShLanguage)stage);
				const char* shaderStrings = source.c_str();
				shader.setStrings(&shaderStrings, 1);

				int ClientInputSemanticsVersion = 100;
				glslang::EShTargetClientVersion VulkanClientVersion = glslang::EShTargetVulkan_1_2;
				glslang::EShTargetLanguageVersion TargetVersion = glslang::EShTargetSpv_1_5;

				shader.setEnvInput(glslang::EShSourceGlsl, (EShLanguage)stage, glslang::EShClientVulkan, ClientInputSemanticsVersion);
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

				std::vector<uint32_t> spirv;
				glslang::GlslangToSpv(*program.getIntermediate((EShLanguage)stage), spirv);

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

	void VulkanShader::Reflect(uint32_t p_Stage, const std::vector<uint32_t>& p_ShaderData)
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

			YM_CORE_TRACE("  {0}", resource.name)
			YM_CORE_TRACE("    Size = {0}", bufferSize)
			YM_CORE_TRACE("    Binding = {0}", binding)
			YM_CORE_TRACE("    Members = {0}", memberCount)
		}

		YM_CORE_TRACE("Input Attributes:")
		for (const auto& resource : resources.stage_inputs)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = (uint32_t)compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = (int)bufferType.member_types.size();

			YM_CORE_TRACE("  {0}", resource.name)
			YM_CORE_TRACE("    Size = {0}", bufferSize)
			YM_CORE_TRACE("    Binding = {0}", binding)
			YM_CORE_TRACE("    Members = {0}", memberCount)
		}
	}

	void VulkanShader::CreateShaderModules()
	{
		m_ShaderStages.resize(m_VulkanSPIRV.size());
		for (auto& [stage, code] : m_VulkanSPIRV)
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size() * sizeof(uint32_t);
			createInfo.pCode = code.data();
			
			VkShaderModule shaderModule;
			auto res = vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule);
			YM_CORE_ASSERT(res == VK_SUCCESS)
			m_ShaderModules[stage] = shaderModule;

			VkPipelineShaderStageCreateInfo ShaderStageInfo{};
			ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			ShaderStageInfo.stage = (VkShaderStageFlagBits)stage;
			ShaderStageInfo.module = m_ShaderModules[stage];
			ShaderStageInfo.pName = "main";
			m_ShaderStages.push_back(ShaderStageInfo);
		}
	}
}