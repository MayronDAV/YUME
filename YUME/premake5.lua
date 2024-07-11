project "YUME"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")


	files
	{
		"Source/**.h",
		"Source/**.cpp",
	}

	includedirs
	{
		"Source",

		"%{IncludeDir.glfw}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.Vulkan}",
		"%{LibraryDir.VulkanSDK}"
	}

	links
	{
		"glfw",

		"%{Library.Vulkan}"
	}

	pchheader "YUME/yumepch.h"
	pchsource "Source/YUME/yumepch.cpp"

	filter "system:windows"
		systemversion "latest"

		defines 
		{
			"YM_PLATFORM_WINDOWS",
			"YM_BUILD_DLL"
		}

	filter "system:linux"
		systemversion "latest"
		defines 
		{
			"YM_PLATFORM_LINUX",
			"YM_BUILD_DLL"
		}

	filter "configurations:Debug"
		defines "YM_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}


	filter "configurations:Release"
		defines "YM_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "YM_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}