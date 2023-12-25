#pragma once

#include <curl_multi.h>
#include <map>

using THTTPCallback = std::function<void(bool, std::string)>;

class CHTTPGet
{
public:
    CHTTPGet();
    ~CHTTPGet();

    CHTTPGet(const CHTTPGet& other) = delete;
    CHTTPGet(CHTTPGet&& other) noexcept;

    void Update();
    void AddRequest(const std::string& uri, std::function<void(bool, std::string)>&& callback);

private:
    curl::curl_multi m_multiHandle;

    std::unordered_map<CURL*, curl::curl_ios<std::ostringstream>*> m_easyStreams;
    std::map<CURL*, std::string> m_requestUris;
    std::map<CURL*, curl::curl_easy*> m_handers;
};
