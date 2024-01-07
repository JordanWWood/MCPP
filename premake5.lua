include "dependencies.lua"

workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }
    startproject "Launcher"

project "Premake"
	kind "Utility"

	targetdir ("%{wks.location}/bin64/%{cfg.buildcfg}/%{prj.name}")

	files {
		"%{wks.location}/**.lua"
	}

	postbuildmessage "Regenerating project files with Premake5!"
	postbuildcommands {
		"%{wks.location}/premake5.exe %{_ACTION} --file=\"%{wks.location}premake5.lua\""
	}

group "MCPP"
    include "src/Common"
    include "src/Network"
    include "src/Core"
    include "src/System"
    include "src/Launcher"
	
group "Libs"
    include "vendor"
group ""
