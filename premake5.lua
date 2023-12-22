

workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }

project "mine"
	kind "StaticLib"
	targetdir "bin64/%{cfg.buildcfg}"
	language "C++"
	files { "vendor/mine/src/**.cc", "vendor/mine/src/**.h" }

project "MCPP"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin64/%{cfg.buildcfg}"
	links { "mine" }
    
    includedirs { "src", "vendor/spdlog/include", "vendor/mine/src" }

    files { "src/**.h", "src/**.cpp" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "RELEASE" }
        optimize "On"

    filter { "platforms:Win64" }
        system "windows"
        architecture "x64"
        toolset "msc"

    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        toolset "gcc"
    