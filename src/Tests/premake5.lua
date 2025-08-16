project "Tests"
    kind "ConsoleApp"
    language "C++"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/tests"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}/tests"

    cppdialect "C++20"
    vectorextensions "AVX2"

    files { "**.cpp", "**.h" }

    includedirs {
        "%{wks.location}/vendor/doctest/doctest",
        "%{wks.location}/src/Common"
    }

    links {
        "Common"
    }

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