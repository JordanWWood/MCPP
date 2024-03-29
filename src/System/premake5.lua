﻿project "System"
    kind "StaticLib"
    language "C++"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
    
    cppdialect "C++20"
        
    pchheader "pch.h"
    pchsource "pch.cpp"
    
    vectorextensions "AVX2"
    
    defines { "CURL_STATICLIB", "TOML_EXCEPTIONS=0" }
    
    includedirs {
        "../System",
        "../Common",
        "../Network",
        "../Core",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.openssl}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.curlcpp}",
        "%{IncludeDir.concurrentqueues}",
        "%{IncludeDir.toml}"
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
        
        includedirs {
            "%{IncludeDir.curl}",
        }
    
    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        toolset "gcc"