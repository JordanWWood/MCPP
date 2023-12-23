workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }

project "zlib"
	kind "StaticLib"
	targetdir "bin64/%{cfg.buildcfg}"
	language "C"
	
	files {	"vendor/zlib/*.c", "vendor/zlib/*.h" }
	
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
	links { "spdlog" }
    
	pchheader "pch.h"
	pchsource "src/pch.cpp"
	
    includedirs { "src", "vendor/spdlog/include", "vendor/json/include", os.getenv("OPENSSL_INSTALL_DIR") .. "/include" }

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
        links { os.getenv("OPENSSL_INSTALL_DIR") .. "/lib/libcrypto_static.lib" }

    filter { "platforms:Linux" }
        system "linux"
        architecture "x64"
        toolset "gcc"
    