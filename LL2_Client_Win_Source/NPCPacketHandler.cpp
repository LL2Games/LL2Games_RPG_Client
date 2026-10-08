#include "NPCPacketHandler.h"

#include "PacketParser.h"
#include "NPC.h"
#include "NPCInteraction.h"
#include "NPCDataManager.h"
#include "stbObject.h"
#include "stbNetworkManager.h"
#include "PlayerManager.h"
#include "stbSceneManager.h"
#include "stbScene.h"

#include <stdexcept>
#include <utility>

NPCPacketHandler::InteractCallback
NPCPacketHandler::s_interactCallback{};

void NPCPacketHandler::HandleNPCSnapshot(const ParsedPacket& packet)
{
    try
    {
        std::size_t offset = 0;
        std::string error;

        int count = 0;

        if (!PacketParser::ParseNextIntField(packet.payload.c_str(),packet.payload.size(),offset,count,error))
        { 
            throw std::runtime_error(error);
        }

        if (count < 0 || count > 1000)
            throw std::runtime_error("invalid NPC count");

        for (int i = 0; i < count; ++i)
        {
            int spawnId = 0;
            int npcId = 0;
            float x = 0.0f;
            float y = 0.0f;
            float interactionRange = 100.0f;

            if (!PacketParser::ParseNextIntField(
                packet.payload.c_str(),
                packet.payload.size(),
                offset,
                spawnId,
                error))
            {
                throw std::runtime_error(error);
            }

            if (!PacketParser::ParseNextIntField(
                packet.payload.c_str(),
                packet.payload.size(),
                offset,
                npcId,
                error))
            {
                throw std::runtime_error(error);
            }

            if (!PacketParser::ParseNextFloatField(
                packet.payload.c_str(),
                packet.payload.size(),
                offset,
                x,
                error))
            {
                throw std::runtime_error(error);
            }

            if (!PacketParser::ParseNextFloatField(
                packet.payload.c_str(),
                packet.payload.size(),
                offset,
                y,
                error))
            {
                throw std::runtime_error(error);
            }

            if (!PacketParser::ParseNextFloatField(
                packet.payload.c_str(),
                packet.payload.size(),
                offset,
                interactionRange,
                error))
            {
                throw std::runtime_error(error);
            }

            const NPCData* npcData =NPCDataManager::getInstance()->FindNPCData(npcId);

            if (npcData == nullptr)
                continue;

            stb::NPC* npc =stb::object::Instantiate<stb::NPC>(stb::enums::eLayerType::Animal,stb::math::Vector2(x, y));

            // 패킷 수신 시점에는 Scene::Initialize()가
            // 이미 끝났으므로 직접 초기화해야 합니다.
            npc->Initialize();

            if (!npc->Setup(spawnId, npcId))
                continue;



            stb::NPCInteraction* interaction =npc->AddComponent<stb::NPCInteraction>();

            interaction->Setup(spawnId,npcId,interactionRange);

            interaction->SetRequestCallback(
                [](int requestedSpawnId)
                {
                    auto* network = stb::NetworkManager::getInstance();
                    auto* player = PlayerManager::getInstance()->GetLocalPlayer();

                    if (!network->IsConnected() || player == nullptr || player->IsDead())
                    {
                        return false;
                    }

                    const int mapId = player->GetPlayerLocation()->mapId;

                    if (mapId <= 0)
                        return false;

                    NPCPacketHandler::SendInteract(mapId, requestedSpawnId);
                    return true;
                });

            interaction->SetRenderInfo(stb::math::Vector2(static_cast<float>(npcData->render.width), static_cast<float>(npcData->render.height)),
                stb::math::Vector2(npcData->render.originX,npcData->render.originY));
        }
    }
    catch (const std::exception& exception)
    {
        OutputDebugStringA("[NPC snapshot] ");
        OutputDebugStringA(exception.what());
        OutputDebugStringA("\n");
    }
}


void NPCPacketHandler::SetInteractCallback(InteractCallback callback)
{
    s_interactCallback = std::move(callback);
}

void NPCPacketHandler::SendInteract(int mapId, int spawnId)
{
    stb::NetworkManager::getInstance()->SendPacket(
        PKT_NPC_INTERACT,
        {
            std::to_string(mapId),
            std::to_string(spawnId)
        });
}

void NPCPacketHandler::HandleInteractResult(const ParsedPacket& packet)
{
    NPCInteractionResult result{};
    std::string failure;

    try
    {
        const char* data = packet.payload.data();
        const size_t payloadSize = packet.payload.size();
        size_t offset = 0;

        std::string error;
        std::string status;

        if (!PacketParser::ParseLengthPrefixedString(
            data, payloadSize, offset, status, error))
        {
            throw std::runtime_error(error);
        }

        if (status == "nok")
        {
            if (!PacketParser::ParseLengthPrefixedString(
                data, payloadSize, offset, failure, error))
            {
                throw std::runtime_error(error);
            }

            if (failure.empty())
                failure = "NPC interaction failed";
        }
        else if (status == "ok")
        {
            int dialogueCount = 0;

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, result.mapId, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, result.spawnId, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, result.npcId, error) ||
                !PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, result.name, error) ||
                !PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, result.role, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, dialogueCount, error))
            {
                throw std::runtime_error(error);
            }

            if (result.mapId <= 0 || result.spawnId <= 0 || result.npcId <= 0 || dialogueCount < 0 || dialogueCount > 100)
            {
                throw std::runtime_error(
                    "invalid NPC interaction data");
            }

            result.dialogue.reserve(dialogueCount);

            for (int i = 0; i < dialogueCount; ++i)
            {
                std::string line;

                if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, line, error))
                {
                    throw std::runtime_error(error);
                }

                result.dialogue.push_back(std::move(line));
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, result.shopId, error))
            {
                throw std::runtime_error(error);
            }

            if (result.shopId < 0)
                throw std::runtime_error("invalid shop ID");
        }
        else
        {
            throw std::runtime_error(
                "invalid NPC interaction status");
        }

        if (offset != payloadSize)
            throw std::runtime_error("unexpected payload fields");
    }
    catch (const std::exception& exception)
    {
        failure = exception.what();
    }

    if (!failure.empty())
    {
        OutputDebugStringA(
            ("[NPC interact] " + failure + "\n").c_str());
    }

    auto* player = PlayerManager::getInstance()->GetLocalPlayer();

    const bool sameMap = player != nullptr && (result.mapId == 0 || result.mapId == player->GetPlayerLocation()->mapId);

    // 성공 응답이 이전 맵의 것이면 무시한다.
    if (!sameMap)
        return;

    auto* scene = stb::SceneManager::getInstance()->GetActiveScene();

    if (scene != nullptr)
    {
        auto* layer = scene->GetLayer(stb::enums::eLayerType::Animal);

        if (layer != nullptr)
        {
            for (auto* object : layer->GetGameObjects())
            {
                if (object == nullptr)
                    continue;

                auto* interaction = object->GetComponent<stb::NPCInteraction>();

                if (interaction != nullptr)
                    interaction->ResetRequest();
            }
        }
    }

    if (s_interactCallback)
    {
        s_interactCallback(failure.empty() ? &result : nullptr,failure);
    }
}
