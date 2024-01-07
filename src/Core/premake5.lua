project "Core"
    kind "StaticLib"
    language "C++"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    
    cppdialect "C++20"
    
    pchheader "pch.h"
    pchsource "pch.cpp"

    includedirs {
        "%{wks.location}/src/Common",
        "%{wks.location}/src/Core",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.uuid}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.json}",
        "%{IncludeDir.concurrentqueues}"
    }
    
    files { "**.h", "**.cpp" }
    
    filter "configurations:Debug"
        defines { "DEBUG", "USE_OPTICK=1" }
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