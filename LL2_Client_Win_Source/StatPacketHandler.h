#pragma once
#include "Packet.h"
#include "CommonInclude.h"


class StatPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandleStatResult(const ParsedPacket& pkt);
	static void SendStatUp(const std::string statType);
};

