#pragma once

#include "Packet.h"
#include "NPC_info.h"

#include <functional>

class NPCPacketHandler
{
public:
    static void HandleNPCSnapshot(const ParsedPacket& packet);
    static void SendInteract(int mapId, int spawnId);
    static void HandleInteractResult(const ParsedPacket& packet);

    // 성공하면 결과 포인터, 실패하면 nullptr.
    // 결과 포인터는 콜백 실행 중에만 유효하다.
    using InteractCallback = std::function<void(const NPCInteractionResult* result, const std::string& error)>;

    static void SetInteractCallback(InteractCallback callback);

private:
    static InteractCallback s_interactCallback;
};
