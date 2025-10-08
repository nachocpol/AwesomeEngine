workspace "AwesomeSolution"
	configurations { "Debug", "Release"}
	platforms "x64"
	systemversion "10"
	staticruntime "on"
	--warnings "Extra"
	flags {"FatalCompileWarnings"}

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
		"Source/Graphics/Shaders",
		"Depen/DX12",
		"Depen/JSON",
		"Depen/TinyObj",
		"Depen/GLM",
		"Depen/STB",
		"Depen/tinyobj"
	}
	files
	{
	}

filter {"configurations:Debug"}
	libdirs
	{
	}
	links
	{
	}
	defines
	{
		"DEBUG"
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
	includedirs
	{		
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