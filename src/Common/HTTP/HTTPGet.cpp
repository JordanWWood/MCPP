#include "pch.h"
#include "HTTPGet.h"

CHTTPGet::CHTTPGet()
{}

CHTTPGet::~CHTTPGet()
{}

CHTTPGet::CHTTPGet(CHTTPGet&& other) noexcept
{
    m_multiHandle = std::move(other.m_multiHandle);
    m_easyStreams = std::move(other.m_easyStreams);
}

void CHTTPGet::Update()
{
    m_multiHandle.perform();

    std::unique_ptr<curl::curl_multi::curl_message> message = m_multiHandle.get_next_finished();
    if(message != nullptr)
    {
        const curl::curl_easy *handler = message->get_handler();
        // Get the stream associated with the curl easy handler.
        
        const auto url = handler->get_info<CURLINFO_EFFECTIVE_URL>();
        const auto response_code = handler->get_info<CURLINFO_RESPONSE_CODE>();
        const auto content_type = handler->get_info<CURLINFO_CONTENT_TYPE>();
        const auto http_code = handler->get_info<CURLINFO_HTTP_CODE>();

        curl::curl_ios<std::ostringstream>* stream_handler = m_easyStreams[handler->get_curl()];
        if (!stream_handler)
        {
            MCLog::error("Recieved message for a request that no longer seems to be valid. CurlPtr[{}]", handler->get_curl());
            return;
        }

        auto content = stream_handler->get_stream()->str();

        MCLog::debug("Code[{}] Type[{}] HttpCode[{}] URL[{}] Content[{}]", response_code.get(), content_type.get(), http_code.get(), url.get(), content);
        
        delete m_handers[handler->get_curl()];
        m_handers.erase(handler->get_curl());
        m_requestUris.erase(handler->get_curl());

        delete stream_handler->get_stream();
        delete stream_handler;

        m_easyStreams.erase(handler->get_curl());
    }
}

void CHTTPGet::AddRequest(const std::string& uri, std::function<void(bool, std::string)>&& callback)
{
    auto *output_stream = new std::ostringstream;
    curl::curl_ios<std::ostringstream>* curl_stream = new curl::curl_ios<std::ostringstream>(*output_stream);
    
    curl::curl_easy* easy = new curl::curl_easy(*curl_stream);
    const auto& [it, success] = m_requestUris.emplace(easy->get_curl(), uri);

    easy->add<CURLOPT_URL>(it->second.c_str());
    easy->add<CURLOPT_FOLLOWLOCATION>(1L);
    easy->add<CURLOPT_SSL_VERIFYPEER>(0);

    m_multiHandle.add(*easy);

    m_easyStreams.emplace(easy->get_curl(), curl_stream);
    m_handers.emplace(easy->get_curl(), easy);
}

