#pragma once

struct IConnection;

enum ESocketFlags : uint8_t
{
    eSF_Bind = BIT(0),
    eSF_Listen = BIT(1),
    eSF_Accept = BIT(2),
    eSF_Connect = BIT(3),
    eSF_Send = BIT(4),
    eSF_Recv = BIT(5),
    eSF_Passive = BIT(6)
};

using TOnPacket = std::function<void(char*, uint32_t)>;
using IConnectionPtr = std::shared_ptr<IConnection>;

class CTCPSocket
{
public:
    CTCPSocket(uint8_t flags, std::string address, uint16_t port)
        : m_flags(flags)
        , m_address(address)
        , m_port(port)
    {}

    CTCPSocket(uint8_t flags, std::string address, uint16_t port, SOCKET socket)
        : m_flags(flags)
        , m_address(address)
        , m_port(port)
        , m_socket(socket)
    {}

    ~CTCPSocket();

    CTCPSocket(const CTCPSocket& other) = delete;
    CTCPSocket& operator=(const CTCPSocket& other) = delete;
    CTCPSocket(CTCPSocket&& other) noexcept;
    CTCPSocket& operator=(CTCPSocket&& other) = default;

    bool Start();
    void Stop();
    
    bool Send(char* data, const uint32_t size);
    bool Recv(TOnPacket&& packetCallback);

    template<class T>
    IConnectionPtr Accept();

    bool IsClosed() const { return m_socket == INVALID_SOCKET; }
    const std::string& GetAddress() const { return m_address; }
    
private:
    uint8_t m_flags;
    std::string m_address;
    uint16_t m_port;
    
    SOCKET m_socket{ INVALID_SOCKET };
};

template <class T>
IConnectionPtr CTCPSocket::Accept()
{
    assert(!(m_flags & (eSF_Send | eSF_Recv)));

    sockaddr_in sa = { 0 }; /* for TCP/IP */
    socklen_t socklen = sizeof sa;

    SOCKET socket = accept(m_socket, reinterpret_cast<struct sockaddr*>(&sa), &socklen);
    if(socket == INVALID_SOCKET)
    {
        if(GET_SOCKET_ERR() == WOULD_BLOCK)
            return nullptr;

        MCLog::error("Failed to accept new client connect with error {}", GET_SOCKET_ERR());

        Stop();
        return nullptr;
    }

    MCLog::info("Accepting connection from {}", inet_ntoa(sa.sin_addr));
    
    return std::make_shared<T>(CTCPSocket(eSF_Send | eSF_Recv, inet_ntoa(sa.sin_addr), 0, socket));
}
