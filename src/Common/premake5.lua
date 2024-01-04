project "Common"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    
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