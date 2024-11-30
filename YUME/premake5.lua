project "YUME"
	kind "SharedLib"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "YUME/yumepch.h"
	pchsource "Source/YUME/yumepch.cpp"

	files
	{
		"Source/**.h",
		"Source/**.cpp",

		"%{IncludeDir.optick}/**.h",
		"%{IncludeDir.optick}/**.cpp",

		"%{IncludeDir.tinyobjloader}/**.h",
		"%{IncludeDir.tinygltf}/**.h",
		"%{IncludeDir.json}/**.hpp",

		"%{IncludeDir.vma}/**.h"
	}

	includedirs
	{
		"Source",

		"%{IncludeDir.glfw}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glslang}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.imgui}/imgui",
		"%{IncludeDir.imgui}/backends",
		"%{IncludeDir.stb}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.vma}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.tinyobjloader}",
		"%{IncludeDir.tinygltf}",
		"%{IncludeDir.json}",

		"%{LibraryDir.VulkanSDK}"
	}

	links
	{
		"glfw",
		"glslang",
		"imgui",
		"stb",

		"%{Library.Vulkan}"
	}

	defines
	{
		"YM_EXPORT",
		"OPTICK_EXPORT"
	}

	filter "files:Thirdparty/optick/src/**.cpp"
		flags { "NoPCH" }

	local source = "%{wks.location}/bin/" .. outputdir .. "/YUME/YUME.dll"
	local destination = "%{wks.location}/bin/" .. outputdir .. "/Sandbox"
	local copyfile = "   {COPYFILE} " .. source .. " " .. destination

	filter "system:windows"
		systemversion "latest"
		buildoptions { "/wd4251" }
		
		defines 
		{
			"YM_PLATFORM_WINDOWS",
			
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
			"YM_PLATFORM_LINUX"
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