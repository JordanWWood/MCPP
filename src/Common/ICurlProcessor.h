#pragma once
#include <functional>
#include <string>

using THTTPCallback = std::function<void(bool, std::string)>;

struct ICurlProcessor
{
    virtual ~ICurlProcessor() = default;
    
    // TODO other http request types
    // Queues a HTTPGet request to the CURL thread. The callback will be called on the CURL thread
    virtual void QueueHttpGet(std::string uri, THTTPCallback&& callback) = 0;
};
