VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["glfw"] = "%{wks.location}/Thirdparty/glfw/include"
IncludeDir["glm"] = "%{wks.location}/Thirdparty/glm/include"
IncludeDir["spdlog"] = "%{wks.location}/Thirdparty/spdlog/include"
IncludeDir["glslang"] = "%{wks.location}/Thirdparty/glslang"
IncludeDir["imgui"] = "%{wks.location}/Thirdparty/imgui"
IncludeDir["stb"] = "%{wks.location}/Thirdparty/stb"
IncludeDir["optick"] = "%{wks.location}/Thirdparty/optick/src"
IncludeDir["vma"] = "%{wks.location}/Thirdparty/VulkanMemoryAllocator/include"
IncludeDir["Vulkan"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
