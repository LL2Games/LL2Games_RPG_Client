#include "NPCPacketHandler.h"

#include "PacketParser.h"
#include "NPC.h"
#include "NPCInteraction.h"
#include "NPCDataManager.h"
#include "stbObject.h"

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

            interaction->SetRenderInfo(
                stb::math::Vector2(
                    static_cast<float>(npcData->render.width),
                    static_cast<float>(npcData->render.height)),
                stb::math::Vector2(
                    npcData->render.originX,
                    npcData->render.originY));
        }
    }
    catch (const std::exception& exception)
    {
        OutputDebugStringA("[NPC snapshot] ");
        OutputDebugStringA(exception.what());
        OutputDebugStringA("\n");
    }
}
