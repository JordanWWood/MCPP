workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }

project "MCPP"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin64/%{cfg.buildcfg}"
    
    includedirs { "src" }

    files { "src/**.h", "src/**.cpp" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "RELEASE" }
        optimize "On"

    filter { "platforms:Win32" }
        system "windows"
        architecture "x86"
        toolset "msc"

    filter { "platforms:Win64" }
        system "windows"
        architecture "x64"
        toolset "msc"

    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        toolset "gcc"
    