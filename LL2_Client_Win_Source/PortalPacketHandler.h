#pragma once
#include "Packet.h"
#include "CommonInclude.h"


class PortalPacketHandler
{
public:
	static void HandleMoveMap(const ParsedPacket& pkt);
	static void SendPortalEnter(std::string portalId);
	

private:
	static void ResetPendingPortal();
	static std::string s_pendingPortalId;
};

