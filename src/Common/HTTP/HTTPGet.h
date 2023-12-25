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

    struct SActiveRequest {
        SActiveRequest() = default;
        ~SActiveRequest();

        std::string m_requestUri;

        curl::curl_ios<std::ostringstream>* m_easyStream{ nullptr };
        curl::curl_easy* m_handler{ nullptr };

        THTTPCallback m_callback{ nullptr };
    };

    std::unordered_map<CURL*, SActiveRequest> m_activeRequests;
};
