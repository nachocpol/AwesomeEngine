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
			"Depen/DX12",
			"Depen/JSON"
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
			"Depen/DX12",
			"Depen/JSON"
		}
		libdirs
		{
		}
		links
		{
		}

project "AwesomeTriangle"
	kind "ConsoleApp"
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

project "AwesomeAdvanced"
	kind "ConsoleApp"
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