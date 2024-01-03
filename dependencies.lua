IncludeDir = {}
IncludeDir["spdlog"] = "%{wks.location}/vendor/spdlog/include"
IncludeDir["optick"] = "%{wks.location}/vendor/optick/src"
IncludeDir["curl"] = "%{wks.location}/vendor/curl/include"
IncludeDir["curlcpp"] = "%{wks.location}/vendor/curlcpp/include"
IncludeDir["uuid"] = "%{wks.location}/vendor/uuid"
IncludeDir["zlib"] = "%{wks.location}/vendor/zlib"
IncludeDir["openssl"] = os.getenv("OPENSSL_INSTALL_DIR") .. "/include"
IncludeDir["json"] = "%{wks.location}/vendor/json/include"

LibraryDir = {}
LibraryDir["openssl"] = os.getenv("OPENSSL_INSTALL_DIR") .. "/lib"

Library = {}
Library["libcrypto"] = "%{LibraryDir.openssl}/libcrypto.lib"
Library["libssl"] = "%{LibraryDir.VulkanSDK}/libssl.lib"

-- Windows
Library["WinSock"] = "ws2_32.lib"
Library["crypt32"] = "crypt32.lib"
Library["wldap32"] = "wldap32.lib"