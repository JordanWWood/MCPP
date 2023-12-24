workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }

project "zlib"
	kind "StaticLib"
	targetdir "bin64/%{cfg.buildcfg}"
	language "C"
	
	files {	"vendor/zlib/*.c", "vendor/zlib/*.h" }
	
project "libcurl"
    kind "StaticLib"
    targetdir "bin64/%{cfg.buildcfg}"
    language "C"
    includedirs { "vendor/curl/include", "vendor/curl/lib", os.getenv("OPENSSL_INSTALL_DIR") .. "/include" }
    defines { "BUILDING_LIBCURL", "CURL_STATICLIB", "USE_OPENSSL", "USE_IPV6" }

    files { "vendor/curl/include/**.h", "vendor/curl/lib/**.h", "vendor/curl/lib/**.c" }
    
    filter "configurations:Debug"
        defines { "DEBUGBUILD" }
        symbols "On"
	
project "curlcpp"
    kind "StaticLib"
    targetdir "bin64/%{cfg.buildcfg}"
    language "C++"
    includedirs { "vendor/curlcpp/include", "vendor/curl/include" }
    
    files { "vendor/curlcpp/src/**.cpp", "vendor/curlcpp/include/**.h" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "RELEASE" }
        optimize "On"
	
project "spdlog"
	kind "StaticLib"
	targetdir "bin64/%{cfg.buildcfg}"
	language "C++"
	includedirs { "vendor/spdlog/include" }
	defines { "SPDLOG_COMPILED_LIB" }
	
	files { "vendor/spdlog/src/**.cpp", "vendor/spdlog/include/**.h" }

project "MCPP"
    kind "ConsoleApp"
    language "C++"
    targetdir "bin64/%{cfg.buildcfg}"
	links { "spdlog", "libcurl", "curlcpp" }
	defines { "CURL_STATICLIB" }
	
	cppdialect "C++20"
    
	pchheader "pch.h"
	pchsource "src/pch.cpp"
	
    includedirs { "src", "vendor/spdlog/include", "vendor/json/include", os.getenv("OPENSSL_INSTALL_DIR") .. "/include", "vendor/curlcpp/include", "vendor/curl/include" }

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
        links { "crypt32.lib", "ws2_32.lib", "wldap32.lib", os.getenv("OPENSSL_INSTALL_DIR") .. "/lib/libcrypto.lib", os.getenv("OPENSSL_INSTALL_DIR") .. "/lib/openssl.lib", os.getenv("OPENSSL_INSTALL_DIR") .. "/lib/libssl.lib"}

    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        toolset "gcc"
    