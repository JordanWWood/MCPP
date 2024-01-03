include "dependencies.lua"

workspace "MCPP"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64", "Linux" }

group "MCPP"
    include "src/Common"
    include "src/Network"
    include "src/Core"
    include "src/Launcher"
	
group "Libs"
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
group ""

group "Tools"
	project "OptickCore"
		kind "SharedLib"
		targetdir "bin64/%{cfg.buildcfg}"
		language "C++"
		defines { "_CRT_SECURE_NO_WARNINGS", "OPTICK_LIB=1", "OPTICK_EXPORTS", "OPTICK_ENABLE_GPU_D3D12=0", "OPTICK_ENABLE_GPU_VULKAN=0" }
	
		includedirs { "vendor/optick/src" }
		
		files {
			"vendor/optick/src/**.cpp",
			"vendor/optick/src/**.h", 
		}
		vpaths {
			["api"] = { 
				"vendor/optick/src/optick.h",
				"vendor/optick/src/optick.config.h",
			},
		}
		
		filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"
group ""
