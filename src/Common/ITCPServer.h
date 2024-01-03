#include <memory>

struct IRSAKeyPair;

struct IConnection;
using IConnectionPtr = std::shared_ptr<IConnection>;

struct ITCPServer
{
    virtual bool Listen() = 0;

    virtual IConnectionPtr AcceptConnection() const = 0;
    virtual bool IsSocketClosed() const = 0;

    virtual std::shared_ptr<IRSAKeyPair> GenerateRSAKeyPair() = 0;
};