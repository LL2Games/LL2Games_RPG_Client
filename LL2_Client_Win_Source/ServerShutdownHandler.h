#pragma once
#include "Packet.h"
#include "CommonInclude.h" 

class ServerShutdownHandler
{
public:
	static void HandleServerShutdown(const ParsedPacket& pkt);
};

