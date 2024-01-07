#include "pch.h"
#include "CurlProcessor.h"

#define CURL_THREAD_UPDATE_RATE 120
using TCurlThreadFrame = std::chrono::duration<int64_t, std::ratio<1, CURL_THREAD_UPDATE_RATE>>;

static void ThreadStart(CCurlProcessor* instance)
{
    OPTICK_THREAD("Curl Thread");
    
    instance->ThreadUpdate();
}

CCurlProcessor::CCurlProcessor()
{
    m_workThread = std::thread(ThreadStart, this);
}

CCurlProcessor::~CCurlProcessor()
{
    m_shutdown = true;
    m_workThread.join();
}

void CCurlProcessor::QueueHttpGet(std::string uri, THTTPCallback&& callback)
{
    m_workQueue.enqueue({ uri, std::move(callback) });
}

void CCurlProcessor::ThreadUpdate()
{
    auto nextFrame = std::chrono::high_resolution_clock::now() + TCurlThreadFrame{1};
    
    while(!m_shutdown)
    {
        SQueuedRequest request;
        if (m_workQueue.try_dequeue(request))
        {
            OPTICK_EVENT("Push New Requests to Curl Multi");
            auto *outputStream = new std::ostringstream;
            std::unique_ptr<curl::curl_ios<std::ostringstream>> curlStream = std::make_unique<curl::curl_ios<std::ostringstream>>(*outputStream);
            std::unique_ptr<curl::curl_easy> easy = std::make_unique<curl::curl_easy>(*curlStream.get());
    
            const auto& [it, success] = m_runningRequests.emplace(easy->get_curl(), std::make_shared<SActiveRequest>());
            it->second->m_callback = std::move(request.callback);

            std::string& requestUri = it->second->m_requestUri;
            requestUri = request.uri;

            easy->add<CURLOPT_URL>(requestUri.c_str());
            easy->add<CURLOPT_FOLLOWLOCATION>(1L);
            easy->add<CURLOPT_SSL_VERIFYPEER>(0);

            m_multiHandle.add(*easy);

            it->second->m_easyStream = std::move(curlStream);
            it->second->m_handler = std::move(easy);
        }
        
        {
            OPTICK_EVENT("Perform Requests");
            m_multiHandle.perform();

            while(std::unique_ptr<curl::curl_multi::curl_message> message = m_multiHandle.get_next_finished())
            {
                const curl::curl_easy *handler = message->get_handler();
                // Get the stream associated with the curl easy handler.
        
                const auto url = handler->get_info<CURLINFO_EFFECTIVE_URL>();
                const auto response_code = handler->get_info<CURLINFO_RESPONSE_CODE>();
                const auto content_type = handler->get_info<CURLINFO_CONTENT_TYPE>();
                const auto http_code = handler->get_info<CURLINFO_HTTP_CODE>();

                SActiveRequest& activeRequest = *m_runningRequests[handler->get_curl()];
                if (!activeRequest.m_easyStream->get_stream())
                {
                    MCLog::error("Recieved message for a request that no longer seems to be valid. CurlPtr[{}]", handler->get_curl());
                    m_runningRequests.erase(handler->get_curl());
                    return;
                }

                auto content = activeRequest.m_easyStream->get_stream()->str();

                MCLog::debug("Code[{}] Type[{}] HttpCode[{}] URL[{}] Content[{}]", response_code.get(), content_type.get(), http_code.get(), url.get(), content);

                activeRequest.m_callback(true, content);
                m_runningRequests.erase(handler->get_curl());
            }
        }

        {
            OPTICK_EVENT("Sleep");
            std::this_thread::sleep_until(nextFrame);
            nextFrame += TCurlThreadFrame{1};
        }
    }
}

CCurlProcessor::SActiveRequest::~SActiveRequest()
{
    if(m_easyStream)
        delete m_easyStream->get_stream();
}
