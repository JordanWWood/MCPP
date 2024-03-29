isVisualStudio = false
if _ACTION == "vs2010" or _ACTION == "vs2012" or _ACTION == "vs2015" or _ACTION == "vs2017" or _ACTION == "vs2022" then
	isVisualStudio = true
end

project "zlib"
	kind "StaticLib"
	targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
	language "C"
	
	files {	"zlib/*.c", "zlib/*.h" }
	
if isVisualStudio then
project "libcurl"
    kind "StaticLib"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    language "C"
    includedirs { "curl/include", "curl/lib", "%{IncludeDir.openssl}" }
    defines { "BUILDING_LIBCURL", "CURL_STATICLIB", "USE_OPENSSL", "USE_IPV6" }

    files { "curl/include/**.h", "curl/lib/**.h", "curl/lib/**.c" }
    
    filter "configurations:Debug"
        defines { "DEBUGBUILD" }
        symbols "On"
end
	
project "curlcpp"
    kind "StaticLib"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
    
    language "C++"
    includedirs { "curlcpp/include", "curl/include" }
    
    files { "curlcpp/src/**.cpp", "curlcpp/include/**.h" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG", "RELEASE" }
        optimize "On"
	
project "spdlog"
	kind "StaticLib"
	targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
	
	language "C++"
	includedirs { "spdlog/include" }
	defines { "SPDLOG_COMPILED_LIB" }
	
	files { "spdlog/src/**.cpp", "spdlog/include/**.h" }
	
project "OptickCore"
	kind "SharedLib"
	targetdir "%{wks.location}/bin64/%{cfg.buildcfg}"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
	
	language "C++"
	defines { "_CRT_SECURE_NO_WARNINGS", "OPTICK_LIB=1", "OPTICK_EXPORTS", "OPTICK_ENABLE_GPU_D3D12=0", "OPTICK_ENABLE_GPU_VULKAN=0" }

if isVisualStudio then
    cppdialect "C++11"
else
	cppdialect "gnu++11"
end

	includedirs { "optick/src" }
	
	files {
		"optick/src/**.cpp",
		"optick/src/**.h", 
	}
	
	vpaths {
		["api"] = { 
			"optick/src/optick.h",
			"optick/src/optick.config.h",
		},
	}
	
	filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"