project "Common"
    kind "StaticLib"
    language "C++"
    targetdir "%{wks.location}/bin64/%{cfg.buildcfg}/libs"
    objdir "%{wks.location}/obj/%{cfg.platform}_%{cfg.buildcfg}"
    
    cppdialect "C++20"
    
    vectorextensions "AVX2"
    
    includedirs {
        "%{wks.location}/src/Common",
        "%{IncludeDir.optick}",
    }
    
    files { "**.cpp", "**.h" }
    
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