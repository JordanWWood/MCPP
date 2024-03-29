﻿project "Launcher"
    kind "ConsoleApp"
    language "C++"
    targetname "MCPP"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
    
    links { 
        "%{Library.libcrypto}",
        "%{Library.libssl}",
        "OptickCore", 
        "spdlog", 
        "curlcpp",
        "Network", 
        "Core",
        "Common",
        "System"
    }
    
    vectorextensions "AVX2"
    
    defines { "CURL_STATICLIB", "MT_INSTRUMENTED_BUILD" }
    cppdialect "C++20"
    
    includedirs {
        "%{wks.location}/src/Common",
        "%{wks.location}/src/System",
        "%{wks.location}/src/Network",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.curlcpp}",
        "%{IncludeDir.curl}"
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
        links { 
            "crypt32.lib", 
            "ws2_32.lib", 
            "wldap32.lib",
            "libcurl"
        }

    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        linkgroups "on"
        toolset "gcc"
        links {
            "%{Library.curl}"
        }