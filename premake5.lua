workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }

project "zlib"
	kind "StaticLib"
	targetdir "bin64/%{cfg.buildcfg}"
	language "C"
	
	files {	"vendor/zlib/*.c", "vendor/zlib/*.h" }

project "mine"
	kind "StaticLib"
	targetdir "bin64/%{cfg.buildcfg}"
	language "C++"
	links {"zlib"}
	includedirs { "vendor/zlib" }
	
	files { "vendor/mine/package/**.h", "vendor/mine/package/**.cc" }
	
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
	links { "mine", "spdlog" }
    
    includedirs { "src", "vendor/spdlog/include", "vendor/mine/package" }

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
    