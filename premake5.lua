workspace "AwesomeSolution"
	configurations { "Debug", "Release"}
	platforms "x64"
	systemversion "10.0.16299.0"
	
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
		includedirs 
		{
			"Source",
			"Depen/DX12"
		}	
		libdirs
		{
		}
		links
		{
		}
	filter "configurations:Release"
		optimize "On"
		includedirs 
		{
			"Source",
			"Depen/DX12"
		}
		libdirs
		{
		}
		links
		{
		}

project "AwesomeTest"
	kind "ConsoleApp"
	language "C++"
	location "Temp/VSFiles"
	targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
	files
	{
		"Source/Test/**.h",
		"Source/Test/**.cpp"
	}
	filter "configurations:Debug"
		symbols "On"
		includedirs 
		{
			"Source",
			"Depen/DX12"
		}	
		libdirs
		{
		}
		links
		{
			"Graphics"
		}
	filter "configurations:Release"
		optimize "On"
		includedirs 
		{
			"Source",
			"Depen/DX12"
		}
		libdirs
		{
		}
		links
		{
			"Graphics"
		}