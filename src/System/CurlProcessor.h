#pragma once

#include <ICurlProcessor.h>

#include <ConcurrentQueue.h>
#include <curl_easy.h>
#include <curl_multi.h>
#include <curl/curl.h>

class CCurlProcessor : public ICurlProcessor
{
public:
    CCurlProcessor();
    ~CCurlProcessor() override;

    CCurlProcessor(const CCurlProcessor& other) = delete;
    CCurlProcessor& operator=(const CCurlProcessor& other) = delete;
    
    CCurlProcessor(CCurlProcessor&& other) = delete;
    CCurlProcessor& operator=(CCurlProcessor&& other) = delete;
    
    void QueueHttpGet(std::string uri, THTTPCallback&& callback) override;
    void ThreadUpdate();
    
private:
    curl::curl_multi m_multiHandle;

    struct SQueuedRequest
    {
        std::string uri;
        THTTPCallback callback;
    };
    
    struct SActiveRequest
    {
        SActiveRequest() = default;
        ~SActiveRequest();

        std::string m_requestUri;

        std::unique_ptr<curl::curl_ios<std::ostringstream>> m_easyStream{ nullptr };
        std::unique_ptr<curl::curl_easy> m_handler{ nullptr };

        THTTPCallback m_callback{ nullptr };
    };

    std::unordered_map<CURL*, std::shared_ptr<SActiveRequest>> m_runningRequests;
    moodycamel::ConcurrentQueue<SQueuedRequest> m_workQueue;

    std::thread m_workThread;
    std::atomic_bool m_shutdown = false;
};
