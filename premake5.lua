workspace "AwesomeSolution"
	configurations { "Debug", "Release"}
	platforms "x64"
	systemversion "10.0.16299.0"

filter { "platforms:x64" }
	defines { "PLATFORM_WINDOWS" }
	includedirs 
	{
		"Source",
		"Depen/DX12",
		"Depen/JSON",
		"Depen/TinyObj",
		"Depen/GLM",
		"Depen/STB",
		"Assets/Shaders",
		"Depen/DirectXTex/DirectXTex",
		"Depen/assimp/include"
	}
	files
	{
		"Assets/Shaders/**.h"
	}

filter {"configurations:Debug"}
	libdirs
	{
		"Depen/DirectXTex/DirectXTex/Bin/Desktop_2017_Win10/x64/Debug",
		"Depen/assimp/lib/Debug"
	}
	links
	{
		"DirectXTex",
		"assimp-vc141-mtd",
		"IrrXMLd",
		"zlibstaticd"
	}

filter {"configurations:Release"}
	libdirs
	{
		"Depen/DirectXTex/DirectXTex/Bin/Desktop_2017_Win10/x64/Release",
		"Depen/assimp/lib/Release"
	}
	links
	{
		"DirectXTex",
		"assimp-vc141-mt",
		"IrrXML",
		"zlibstatic"
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
			"copy %{wks.location}Depen\\assimp\\bin\\%{cfg.buildcfg}\\assimp-vc141-mtd.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\assimp-vc141-mtd.dll"
		}
	filter "configurations:Release"
		optimize "On"
		links
		{
			"Graphics", "Core"
		}
		postbuildcommands 
		{
			"copy %{wks.location}Depen\\assimp\\bin\\%{cfg.buildcfg}\\assimp-vc141-mt.dll %{wks.location}Build\\%{cfg.platform}\\%{cfg.buildcfg}\\assimp-vc141-mt.dll"
		}

--[[
project "AwesomeShowcase"
	kind "ConsoleApp"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/Samples/**.h",
		"Source/Samples/Showcase.cpp",
		"Source/Samples/ShowcaseScene.cpp"
	}
	filter "configurations:Debug"
		symbols "On"
		libdirs
		{
			"../assimp/lib/Debug"
		}
		links
		{
			"Graphics", "Core",
			"assimp-vc140-mt"
		}
	filter "configurations:Release"
		optimize "On"
		libdirs
		{
			"../assimp/lib/Release"
		}
		links
		{
			"Graphics", "Core",
			"assimp-vc140-mt"
		}
--]]