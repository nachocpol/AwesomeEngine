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
		"../DirectXTex/DirectXTex",
		"../assimp/include"
	}
	files
	{
		"Assets/Shaders/**.h"
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
	
		libdirs
		{
			"../DirectXTex/DirectXTex/Bin/Desktop_2017_Win10/x64/Debug",
			"../assimp/lib/Debug"
		}
		links
		{
			"DirectXTex",
			"assimp-vc140-mt",
			"IrrXML",
			"zlibstaticd"
		}
	filter "configurations:Release"
		optimize "On"
		libdirs
		{
			"../DirectXTex/DirectXTex/Bin/Desktop_2017_Win10/x64/Release",
			"../assimp/lib/Release"
		}
		links
		{
			"DirectXTex",
			"assimp-vc140-mt"
		}

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
		libdirs
		{
			"../DirectXTex/DirectXTex/Bin/Desktop_2017_Win10/x64/Debug",
			"../assimp/lib/Debug"
		}
		links
		{
			"DirectXTex",
			"assimp-vc140-mt",
			"IrrXML",
			"zlibstaticd",
			"Core"
		}
	filter "configurations:Release"
		optimize "On"
		libdirs
		{
			"../DirectXTex/DirectXTex/Bin/Desktop_2017_Win10/x64/Release",
			"../assimp/lib/Release"
		}
		links
		{
			"DirectXTex",
			"assimp-vc140-mt"
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
		libdirs
		{
		}
		links
		{
			"Graphics", "Core"
		}
	filter "configurations:Release"
		optimize "On"
		libdirs
		{
		}
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
		libdirs
		{
		}
		links
		{
			"Graphics", "Core"
		}
	filter "configurations:Release"
		optimize "On"
		libdirs
		{
		}
		links
		{
			"Graphics", "Core"
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