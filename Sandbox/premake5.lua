project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")


	files
	{
		"Source/**.h",
		"Source/**.cpp"
	}

	includedirs
	{
		"Source",
		"%{wks.location}/YUME/Source",

		"%{IncludeDir.glm}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.imgui}/imgui",
		"%{IncludeDir.imgui}/backends",
		"%{IncludeDir.spdlog}"
	}

	links 
	{
		"YUME",
		"imgui"
	}

	postbuildcommands 
	{
		("{COPYFILE} %{wks.location}/bin/" .. outputdir .. "/YUME/YUME.dll %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/")
	}

	filter "system:windows"
		systemversion "latest"
		defines "YM_PLATFORM_WINDOWS"

	filter "system:linux"
		defines "YM_PLATFORM_LINUX"

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