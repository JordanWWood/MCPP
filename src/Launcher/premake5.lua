project "Launcher"
    kind "ConsoleApp"
    language "C++"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}"
    links { 
        "OptickCore", 
        "spdlog", 
        "libcurl", 
        "curlcpp",
        "Network", 
        "Core" 
    }
    
    defines { "CURL_STATICLIB", "MT_INSTRUMENTED_BUILD" }
    cppdialect "C++20"
    
    includedirs { 
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.curlcpp}",
        "%{IncludeDir.curl}"
    }
    
    includedirs {
        "%{wks.location}/src/Core",
        "%{wks.location}/src/Common",
        "%{wks.location}/src/Network"
    }

    files { "**.h", "**.cpp", "*.lua" }

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
            os.getenv("OPENSSL_INSTALL_DIR") .. "/lib/libcrypto.lib",
            os.getenv("OPENSSL_INSTALL_DIR") .. "/lib/libssl.lib"
        }

    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        toolset "gcc"