#pragma once

#include <functional>
#include "IConnection.h"

struct IRSAKeyPair;
struct IPacketHandler;

struct INetwork
{
    virtual ~INetwork() = default;

    virtual void RegisterPacketHandler(std::weak_ptr<IPacketHandler> handlerWeakPtr) = 0;

    virtual void RegisterConnectionCallback(void* pCreator, std::function<void(const IConnectionPtr& pConnection)>&& functor) = 0;
    virtual void UnregisterConnectionCallback(void* creator) = 0;

    virtual bool HasShutdown() const = 0;

    virtual std::shared_ptr<IRSAKeyPair> GetServerKeyPair() = 0;
    virtual std::string GenerateHexDigest(std::string publicKey, std::string sharedSecret) = 0;
};
