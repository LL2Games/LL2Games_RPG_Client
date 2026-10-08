#include "MovementPacketHandler.h"
#include "MovementProtocol.h"
#include "PacketParser.h"
#include "stbNetworkManager.h"
#include "PlayerManager.h"
#include "stbPlayer.h"
#include "stbOtherPlayerManager.h"
#include "MonsterManager.h"
#include "ProjectileManager.h"
#include <chrono>
#include <map>
#include <mutex>
#include <tuple>

namespace
{
    using Clock = std::chrono::steady_clock;
    using Key = std::tuple<int, int, int>;
    struct Pending { movement::Snapshot value; Clock::time_point received; };
    std::mutex inboxMutex;
    std::map<Key, Pending> inbox;
    int activeMap = 0;
    bool transitioning = false;
    bool resetConnection = false;
    constexpr std::size_t MaxPending = 1024;
    void Queue(const Pending& pending)
    {
        const auto& s = pending.value;
        std::lock_guard<std::mutex> lock(inboxMutex);
        if (activeMap != 0 && !transitioning && s.mapId != activeMap) return;
        const Key key{s.mapId, static_cast<int>(s.kind), s.entityId};
        auto it = inbox.find(key);
        if (it != inbox.end())
        {
            if (movement::IsNewer(s, it->second.value)) it->second = pending;
        }
        else if (inbox.size() < MaxPending) inbox.emplace(key, pending);
    }
}
void MovementPacketHandler::Execute(const ParsedPacket& packet)
{
    movement::Snapshot snapshot;
    std::string error;
    if (!movement::ParseSnapshot(packet.payload, snapshot, error))
    {
        OutputDebugStringA(("[Movement rejected] " + error + "\n").c_str());
        return;
    }
    Queue({snapshot, Clock::now()});
}
void MovementPacketHandler::HandleInputResponse(const ParsedPacket& packet)
{
    std::size_t offset = 0;
    std::string status, reason, error;
    if (!PacketParser::ParseLengthPrefixedString(packet.payload.data(), packet.payload.size(), offset, status, error)) return;
    if (status == "nok")
    {
        PacketParser::ParseLengthPrefixedString(packet.payload.data(), packet.payload.size(), offset, reason, error);
        OutputDebugStringA(("[Movement input rejected] " + reason + "\n").c_str());
    }
}
bool MovementPacketHandler::SendInput(int mapId, int epoch, int sequence, const movement::Input& input)
{
    auto* network = stb::NetworkManager::getInstance();
    std::vector<std::string> fields;
    if (!network || !network->IsConnected() || !movement::BuildInputFields(mapId, epoch, sequence, input, fields)) return false;
    network->SendPacket(PKT_MOVEMENT_INPUT, fields);
#ifdef _DEBUG
    OutputDebugStringA(("[Movement input] map=" + std::to_string(mapId) + " epoch=" + std::to_string(epoch) +
        " sequence=" + std::to_string(sequence) + " axes=" + std::to_string(input.horizontal) + "," +
        std::to_string(input.vertical) + " jump=" + (input.jump ? "1\n" : "0\n")).c_str());
#endif
    return true;
}
void MovementPacketHandler::Pump()
{
    std::map<Key, Pending> batch;
    bool reset = false;
    int mapId = 0;
    {
        std::lock_guard<std::mutex> lock(inboxMutex);
        reset = resetConnection; resetConnection = false; mapId = activeMap;
        if (!transitioning) batch.swap(inbox);
    }
    auto* player = PlayerManager::getInstance()->GetLocalPlayer();
    if (reset && player) player->GetMovementScript()->ResetMovementConnection();
    if (reset)
    {
        ProjectileManager::getInstance()->Clear("disconnect");
        for (auto& entry : stb::OtherPlayerManager::getInstance()->GetPlayers())
            if (entry.second) entry.second->ResetMovementConnection();
        MonsterManager::getInstance()->ResetMovementConnections();
    }
    for (const auto& entry : batch)
    {
        const auto& pending = entry.second;
        const auto& s = pending.value;
        if (Clock::now() - pending.received > std::chrono::seconds(5)) continue;
        if (mapId != 0 && s.mapId != mapId) continue;
        if (!player || player->GetPlayerIdentity()->charId == 0 || mapId == 0)
        {
            Queue(pending); continue;
        }
        bool found = false;
        if (s.kind == movement::Kind::Player)
        {
            if (s.entityId == player->GetPlayerIdentity()->charId)
            {
                player->GetMovementScript()->ApplyMovementSnapshot(s); found = true;
            }
            else
            {
                auto& players = stb::OtherPlayerManager::getInstance()->GetPlayers();
                auto it = players.find(std::to_string(s.entityId));
                if (it != players.end() && it->second)
                {
                    it->second->ApplyMovementSnapshot(s); found = true;
                }
            }
        }
        else if (auto* monster = MonsterManager::getInstance()->FindMonster(s.entityId))
        {
            monster->ApplyMovementSnapshot(s); found = true;
        }
        if (!found) Queue(pending);
    }
}
void MovementPacketHandler::BeginMapTransition()
{
    if (auto* player = PlayerManager::getInstance()->GetLocalPlayer()) player->GetMovementScript()->SuspendMovement();
    std::lock_guard<std::mutex> lock(inboxMutex);
    inbox.clear(); transitioning = true;
}
void MovementPacketHandler::CommitMap(int mapId)
{
    std::lock_guard<std::mutex> lock(inboxMutex);
    activeMap = mapId; transitioning = false;
    for (auto it = inbox.begin(); it != inbox.end();)
        if (it->second.value.mapId != mapId) it = inbox.erase(it);
        else ++it;
}
void MovementPacketHandler::CancelMapTransition()
{
    if (auto* player = PlayerManager::getInstance()->GetLocalPlayer()) player->GetMovementScript()->CancelMovementSuspend();
    std::lock_guard<std::mutex> lock(inboxMutex);
    transitioning = false;
    for (auto it = inbox.begin(); it != inbox.end();)
        if (it->second.value.mapId != activeMap) it = inbox.erase(it);
        else ++it;
}
void MovementPacketHandler::ResetConnection()
{
    std::lock_guard<std::mutex> lock(inboxMutex);
    inbox.clear(); activeMap = 0; transitioning = false; resetConnection = true;
}
void MovementPacketHandler::Forget(movement::Kind kind, int entityId)
{
    std::lock_guard<std::mutex> lock(inboxMutex);
    for (auto it = inbox.begin(); it != inbox.end();)
        if (it->second.value.kind == kind && it->second.value.entityId == entityId) it = inbox.erase(it);
        else ++it;
}
