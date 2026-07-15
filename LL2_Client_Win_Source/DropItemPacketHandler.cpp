#include "DropItemPacketHandler.h"
#include "PacketParser.h"
#include "InventoryManager.h"
#include "Inventory_info.h"
#include "stbSceneManager.h"
#include "DropItemManager.h"

#define M_INVENTORYMANAGER stb::SingletonBase<InventoryManager>::getInstance()
#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()

void DropItemPacketHandler::HandleSpawnDropItem(const ParsedPacket& pkt)
{
   
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;
        int dropSize = 0;
        
        DropItemManager* dropItemManager = M_SCENEMANAGER->GetActiveScene()->GetDropItemManager();
        
        if (dropItemManager == nullptr)
        {
            throw std::runtime_error("dropItemManager is nullptr");
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, dropSize, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        for (int i = 0; i < dropSize; i++)
        {
            DropItemData dropItemData;

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, dropItemData.dropId, errMsg))
            {
                throw std::runtime_error(errMsg);
            }


            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, dropItemData.itemId, errMsg))
            {
                throw std::runtime_error(errMsg);
            }


            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, dropItemData.count, errMsg))
            {
                throw std::runtime_error(errMsg);
            }


            if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, dropItemData.itemPos.x, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, dropItemData.itemPos.y, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            dropItemManager->AddDropItem(dropItemData);
        }

        OutputDebugStringA("Item register Sucess\n");
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

void DropItemPacketHandler::HandleRemoveDropItem(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;
        int removeItemSize = 0;
        int removeDropId = 0;

        DropItemManager* dropItemManager = M_SCENEMANAGER->GetActiveScene()->GetDropItemManager();

        if (dropItemManager == nullptr)
        {
            throw std::runtime_error("dropItemManager is nullptr");
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, removeItemSize, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        for (int i = 0; i < removeItemSize; i++)
        {
            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, removeDropId, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            dropItemManager->RemoveDropItem(removeDropId);
        }

        OutputDebugStringA("Remove Item Sucess\n");
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
