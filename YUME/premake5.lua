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
		"%{IncludeDir.spdlog}"
	}

	links
	{
		"GLFW"
	}

	postbuildcommands 
	{
		("{COPYFILE} %{cfg.buildtarget.relpath} %{wks.location}/bin/" .. outputdir .. "/Sandbox/")
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "on"

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

	filter "configurations:Release"
		defines "YM_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "YM_DIST"
		runtime "Release"
		optimize "on"