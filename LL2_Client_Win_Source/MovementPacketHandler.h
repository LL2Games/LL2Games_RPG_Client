#pragma once
#include "Packet.h"
#include "MovementTypes.h"
class MovementPacketHandler
{
public:
    static void Execute(const ParsedPacket& packet);
    static void HandleInputResponse(const ParsedPacket& packet);
    static void Pump();
    static bool SendInput(int mapId, int epoch, int sequence, const movement::Input& input);
    static void BeginMapTransition();
    static void CommitMap(int mapId);
    static void CancelMapTransition();
    static void ResetConnection();
    static void Forget(movement::Kind kind, int entityId);
};
