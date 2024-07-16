include "dependencies.lua"

workspace "YUME"
	architecture "x64"
	startproject "Sandbox"
	
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	filter "system:windows"
		flags { "MultiProcessorCompile" }
		defines
		{
			"_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING",
			"_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS"
		}
	
	
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Thirdparty/premake"
	include "Thirdparty/glfw"
	include "Thirdparty/glm"
	include "Thirdparty/spdlog"
	include "Thirdparty/glslang"
group ""

group "Core"
	include "YUME"
group ""

group "Misc"
	include "Sandbox"
group ""