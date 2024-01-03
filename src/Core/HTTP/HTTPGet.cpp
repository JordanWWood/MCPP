#include "pch.h"
#include "HTTPGet.h"

CHTTPGet::CHTTPGet()
{}

CHTTPGet::~CHTTPGet()
{}

CHTTPGet::CHTTPGet(CHTTPGet&& other) noexcept
{
    m_multiHandle = std::move(other.m_multiHandle);
    m_activeRequests = std::move(other.m_activeRequests);
}

CHTTPGet& CHTTPGet::operator=(CHTTPGet&& other) noexcept
{
    m_multiHandle = std::move(other.m_multiHandle);
    m_activeRequests = std::move(other.m_activeRequests);

    return *this;
}

void CHTTPGet::Update()
{
    OPTICK_EVENT();

    m_multiHandle.perform();

    while(std::unique_ptr<curl::curl_multi::curl_message> message = m_multiHandle.get_next_finished())
    {
        const curl::curl_easy *handler = message->get_handler();
        // Get the stream associated with the curl easy handler.
        
        const auto url = handler->get_info<CURLINFO_EFFECTIVE_URL>();
        const auto response_code = handler->get_info<CURLINFO_RESPONSE_CODE>();
        const auto content_type = handler->get_info<CURLINFO_CONTENT_TYPE>();
        const auto http_code = handler->get_info<CURLINFO_HTTP_CODE>();

        SActiveRequest& activeRequest = *m_activeRequests[handler->get_curl()];
        if (!activeRequest.m_easyStream->get_stream())
        {
            MCLog::error("Recieved message for a request that no longer seems to be valid. CurlPtr[{}]", handler->get_curl());
            m_activeRequests.erase(handler->get_curl());
            return;
        }

        auto content = activeRequest.m_easyStream->get_stream()->str();

        MCLog::debug("Code[{}] Type[{}] HttpCode[{}] URL[{}] Content[{}]", response_code.get(), content_type.get(), http_code.get(), url.get(), content);

        activeRequest.m_callback(true, content);
        m_activeRequests.erase(handler->get_curl());
    }
}

void CHTTPGet::AddRequest(const std::string& uri, std::function<void(bool, std::string)>&& callback)
{
    OPTICK_EVENT();

    auto *outputStream = new std::ostringstream;
    std::unique_ptr<curl::curl_ios<std::ostringstream>> curlStream = std::make_unique<curl::curl_ios<std::ostringstream>>(*outputStream);
    std::unique_ptr<curl::curl_easy> easy = std::make_unique<curl::curl_easy>(*curlStream.get());
    
    const auto& [it, success] = m_activeRequests.emplace(easy->get_curl(), std::make_shared<SActiveRequest>());
    it->second->m_callback = std::move(callback);

    std::string& requestUri = it->second->m_requestUri;
    requestUri = uri;

    easy->add<CURLOPT_URL>(requestUri.c_str());
    easy->add<CURLOPT_FOLLOWLOCATION>(1L);
    easy->add<CURLOPT_SSL_VERIFYPEER>(0);

    m_multiHandle.add(*easy);

    it->second->m_easyStream = std::move(curlStream);
    it->second->m_handler = std::move(easy);
}

CHTTPGet::SActiveRequest::~SActiveRequest()
{
    if(m_easyStream)
        delete m_easyStream->get_stream();
}
