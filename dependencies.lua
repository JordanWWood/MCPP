IncludeDir = {}
Library = {}
LibraryDir = {}

IncludeDir["spdlog"] = "%{wks.location}/vendor/spdlog/include"
IncludeDir["optick"] = "%{wks.location}/vendor/optick/src"
IncludeDir["curlcpp"] = "%{wks.location}/vendor/curlcpp/include"
IncludeDir["zlib"] = "%{wks.location}/vendor/zlib"
IncludeDir["toml"] = "%{wks.location}/vendor/tomlplusplus/include"

if _ACTION ~= "gmake" then
    OpenSSLPath = os.getenv("VCPKG_INSTALLATION_ROOT") .. "/packages/openssl_x64-windows-static"
    
    IncludeDir["openssl"] = OpenSSLPath .. "/include"
    IncludeDir["curl"] = "%{wks.location}/vendor/curl/include"
    
    LibraryDir["openssl"] = OpenSSLPath .. "/lib"
    Library["libcrypto"] = "%{LibraryDir.openssl}/libcrypto.lib "
    Library["libssl"] = "%{LibraryDir.openssl}/libssl.lib"
else
    IncludeDir["openssl"] = "/usr/include/openssl"
    Library["libcrypto"] = "crypto"
    Library["libssl"] = "ssl"
    Library["curl"] = "curl"
end
    

IncludeDir["json"] = "%{wks.location}/vendor/json/include"
IncludeDir["concurrentqueues"] = "%{wks.location}/vendor/concurrentqueues"

-- Windows Only
Library["WinSock"] = "ws2_32.lib"
Library["crypt32"] = "crypt32.lib"
Library["wldap32"] = "wldap32.lib"
