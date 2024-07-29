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
		"%{IncludeDir.glslang}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.Vulkan}",
		"%{LibraryDir.VulkanSDK}"
	}

	links
	{
		"glfw",
		"glslang",
		"imgui",

		"%{Library.Vulkan}"
	}

	pchheader "YUME/yumepch.h"
	pchsource "Source/YUME/yumepch.cpp"

	local source = "%{wks.location}/bin/" .. outputdir .. "/YUME/YUME.dll"
	local destination = "%{wks.location}/bin/" .. outputdir .. "/Sandbox"
	local copyfile = "   {COPYFILE} " .. source .. " " .. destination

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/wd4251" }
		defines 
		{
			"YM_PLATFORM_WINDOWS",
			"YM_BUILD_DLL"
		}

		postbuildcommands
		{
			("if exist " .. destination .. " ("),
			(copyfile),
			(")")
		}

	filter "system:linux"
		systemversion "latest"
		buildoptions { "-Wno-effc++" }
		defines 
		{
			"YM_PLATFORM_LINUX",
			"YM_BUILD_DLL"
		}

		postbuildcommands 
		{
			("if [ -f " .. destination .. " ]; then \\"),
			(copyfile .. "; \\"),
			("fi")
		}


	filter "configurations:Debug"
		defines "YM_DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
		}


	filter "configurations:Release"
		defines "YM_RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
		}

	filter "configurations:Dist"
		defines "YM_DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
		}