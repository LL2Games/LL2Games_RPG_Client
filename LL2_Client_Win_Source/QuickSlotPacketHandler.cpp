#include "QuickSlotPacketHandler.h"
#include "PlayerManager.h"
#include "PacketParser.h"
#include "QuickSlotManager.h"
#include "stbNetworkManager.h"

#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

void QuickSlotPacketHandler::HandleSlotList(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;

        auto player = M_PLAYERMANAGER->GetLocalPlayer();
        if (player == nullptr)
        {
            throw std::runtime_error("player is nullptr");
            
        }

        auto quickSlotManager = player->GetQuickSlotManager();

        int quickSlotSize = 0;
        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotSize, errMsg))
        {
            throw std::runtime_error(errMsg);
        }
       
        for (int i = 0; i < quickSlotSize; i++)
        {
            QuickSlotData quickSlotData;
            int type=0;

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.slot_index, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, type, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            quickSlotData.type = QuickSlot::SetQuickSlotType(type);

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.ref_id, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            int inventoryType = 0;
            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventoryType, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            quickSlotData.inventory_type = inventoryConvert::SetInvenType(inventoryType);

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.inventory_slotPos, errMsg))
            {
                throw std::runtime_error(errMsg);
            }
            quickSlotManager->SetSlot(quickSlotData);
        }
        OutputDebugStringA("QuickSlotManager Success\n");
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러 발생\n");
    }
}

void QuickSlotPacketHandler::HandleSlotSet(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;
        std::string DebugMsg;
        int size = 0;

        auto player = M_PLAYERMANAGER->GetLocalPlayer();
        if (player == nullptr)
        {
            throw std::runtime_error("player is nullptr");
        }

        auto quickSlotManager = player->GetQuickSlotManager();

      
        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, size, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        for (int i = 0; i < size; i++)
        {
            QuickSlotData quickSlotData;
            int type = 0;
            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.slot_index, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, type, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            quickSlotData.type = QuickSlot::SetQuickSlotType(type);


            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.ref_id, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            int inventoryType = 0;
            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventoryType, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            quickSlotData.inventory_type = inventoryConvert::SetInvenType(inventoryType);

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.inventory_slotPos, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, quickSlotData.count, errMsg))
            {
                throw std::runtime_error(errMsg);
            }
         
            quickSlotManager->SetSlot(quickSlotData);

            OutputDebugStringA("QuickSlotManager Success\n");
        }
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러 발생\n");
    }
}

void QuickSlotPacketHandler::SendSetQuickSlot(const QuickSlotData& quickSlotData)
{
    std::vector<std::string> data;

    data.push_back(std::to_string(quickSlotData.slot_index));
    data.push_back(std::to_string(static_cast<int>(quickSlotData.type)));
    data.push_back(std::to_string(quickSlotData.ref_id));
    data.push_back(std::to_string(static_cast<int>(quickSlotData.inventory_type)));
    data.push_back(std::to_string(quickSlotData.inventory_slotPos));
    data.push_back(std::to_string(quickSlotData.count));

    stb::NetworkManager::getInstance()->SendPacket(PKT_QUICKSLOT_SET, data);
    OutputDebugStringA("[PKT_QUICKSLOT_SET 전송 완료]\n\n");
}
