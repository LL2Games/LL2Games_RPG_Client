#pragma once
#include "Packet.h"
#include <functional>
class IPacketHandler
{
public:
    virtual ~IPacketHandler() = default;
    virtual void Execute(PacketContext* ctx) = 0;
    
};