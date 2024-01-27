IncludeDir = {}
IncludeDir["spdlog"] = "%{wks.location}/vendor/spdlog/include"
IncludeDir["optick"] = "%{wks.location}/vendor/optick/src"
IncludeDir["curlcpp"] = "%{wks.location}/vendor/curlcpp/include"
IncludeDir["zlib"] = "%{wks.location}/vendor/zlib"

if _ACTION ~= "gmake" then
	IncludeDir["openssl"] = os.getenv("OPENSSL_INSTALL_DIR") .. "/include"
	IncludeDir["curl"] = "%{wks.location}/vendor/curl/include"
else
	IncludeDir["openssl"] = "/usr/include/openssl"
end

IncludeDir["json"] = "%{wks.location}/vendor/json/include"
IncludeDir["concurrentqueues"] = "%{wks.location}/vendor/concurrentqueues"

LibraryDir = {}
if _ACTION ~= "gmake" then
    LibraryDir["openssl"] = os.getenv("OPENSSL_INSTALL_DIR") .. "/lib"
end

Library = {}
if _ACTION ~= "gmake" then
    Library["libcrypto"] = "%{LibraryDir.openssl}/libcrypto.lib "
    Library["libssl"] = "%{LibraryDir.openssl}/libssl.lib"
else
   Library["libcrypto"] = "crypto"
   Library["libssl"] = "ssl"
   Library["curl"] = "curl"
end

-- Windows
Library["WinSock"] = "ws2_32.lib"
Library["crypt32"] = "crypt32.lib"
Library["wldap32"] = "wldap32.lib"
