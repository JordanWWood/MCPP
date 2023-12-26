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
    CHTTPGet& operator=(const CHTTPGet& other) = delete;
    
    CHTTPGet(CHTTPGet&& other) noexcept;
    CHTTPGet& operator=(CHTTPGet&& other) noexcept;

    void Update();
    void AddRequest(const std::string& uri, std::function<void(bool, std::string)>&& callback);
    bool IsComplete() const { return m_activeRequests.empty(); }

private:
    curl::curl_multi m_multiHandle;

    struct SActiveRequest {
        SActiveRequest() = default;
        ~SActiveRequest();

        std::string m_requestUri;

        std::unique_ptr<curl::curl_ios<std::ostringstream>> m_easyStream{ nullptr };
        std::unique_ptr<curl::curl_easy> m_handler{ nullptr };

        THTTPCallback m_callback{ nullptr };
    };

    std::unordered_map<CURL*, std::shared_ptr<SActiveRequest>> m_activeRequests;
};
