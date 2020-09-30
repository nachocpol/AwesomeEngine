workspace "AwesomeSolution"
	configurations { "Debug", "Release"}
	platforms "x64"
	systemversion "10"
	staticruntime "on"

filter { "platforms:x64" }
	defines { "PLATFORM_WINDOWS" }
	disablewarnings 
	{
		"4006",
		"4221"
	}
	includedirs 
	{
		"Source",
		"Depen/DX12",
		"Depen/JSON",
		"Depen/TinyObj",
		"Depen/GLM",
		"Depen/STB",
		"Assets/Shaders"
	}
	files
	{
		"Assets/Shaders/**.h"
	}

filter {"configurations:Debug"}
	libdirs
	{
	}
	links
	{
	}

filter {"configurations:Release"}
	libdirs
	{
	}
	links
	{
	}
	defines
	{
		"NDEBUG"
	}

project "Core"
	kind "StaticLib"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/Core/**.h",
		"Source/Core/**.cpp"
	}
	filter "configurations:Debug"
		symbols "On"
	
	filter "configurations:Release"
		optimize "On"

project "Graphics"
	kind "StaticLib"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/Graphics/**.h",
		"Source/Graphics/**.cpp"
	}
	filter "configurations:Debug"
		symbols "On"
		links
		{
			"Core"
		}
	filter "configurations:Release"
		optimize "On"
		links
		{
			"Core"
		}

project "AwesomeTriangle"
	kind "WindowedApp"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/Samples/**.h",
		"Source/Samples/TriangleSample.cpp"
	}
	filter "configurations:Debug"
		symbols "On"
		links
		{
			"Graphics", "Core"
		}
	filter "configurations:Release"
		optimize "On"
		links
		{
			"Graphics", "Core"
		}

project "AwesomeAdvanced"
	kind "WindowedApp"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/Samples/**.h",
		"Source/Samples/AdvancedSample.cpp"
	}
	filter "configurations:Debug"
		symbols "On"
		links
		{
			"Graphics", "Core"
		}
		postbuildcommands 
		{
		}
	filter "configurations:Release"
		optimize "On"
		links
		{
			"Graphics", "Core"
		}
		postbuildcommands 
		{
		}