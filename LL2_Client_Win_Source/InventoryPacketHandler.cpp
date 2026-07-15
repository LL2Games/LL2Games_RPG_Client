#include "InventoryPacketHandler.h"
#include "PacketParser.h"
#include "StringConvert.h"
#include "InventoryManager.h"
#include "stbNetworkManager.h"
#include "UIManager.h"
#include "InventoryUI.h"

#define M_INVENTORYMANAGER stb::SingletonBase<InventoryManager>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()

void InventoryPacketHandler::Execute(const ParsedPacket& pkt)
{
	(void)pkt;
}

void InventoryPacketHandler::HandleInventoryMetaInfo(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;

        int metaInfoSize = 0;
        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, metaInfoSize, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        for (int i = 0; i < metaInfoSize; i++)
        {
            InventoryMetaInfo inventoryMetaInfo;

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventoryMetaInfo.inventoryType, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventoryMetaInfo.max_slots, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventoryMetaInfo.currnet_slots_size, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!M_INVENTORYMANAGER->CreateInventory(inventoryMetaInfo))
            {
                throw std::runtime_error("CreateInventory Failed");
            }
        }
        OutputDebugStringA("Inventory Create Sucess\n");
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

void InventoryPacketHandler::HandleInventoryItemInfo(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;
        int itemSize = 0;
        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, itemSize, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        M_INVENTORYMANAGER->ClearItems();
  
        for (int i = 0; i < itemSize; i++)
        {
            InventoryItemInfo ItemInfo;

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, ItemInfo.inventoryType, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

  
            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, ItemInfo.itemId, errMsg))
            {
                throw std::runtime_error(errMsg);
            }


            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, ItemInfo.itemCount, errMsg))
            {
                throw std::runtime_error(errMsg);
            }


            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, ItemInfo.slotPos, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

       
            auto inventory = M_INVENTORYMANAGER->GetInventory(ItemInfo.inventoryType);
            if (!inventory->SetSlot(ItemInfo))
            {
                throw std::runtime_error("Inventory SetSlot Failed");
            }
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

void InventoryPacketHandler::HandleInventoryMoveItem(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;
        int result = 0;
        int inventoryType = 0;
        int updateSize = 0;
        
        std::vector<InventorySlotUpdate> updatedSlots;

      
        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, result, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (result != 1)
        {
            throw std::runtime_error("인벤토리 아이템 슬롯 변경 실패");
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventoryType, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, updateSize, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        Inventory* inventory = M_INVENTORYMANAGER->GetInventory(inventoryType);
        if (inventory == nullptr)
        {
            throw std::runtime_error("inventory is nullptr");
        }

        for (int i = 0; i < updateSize; i++)
        {
            InventorySlotUpdate inventorySlotUpdate{};

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventorySlotUpdate.slotPos, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventorySlotUpdate.itemId, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, inventorySlotUpdate.itemCount, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            inventory->SetSlot(
                inventorySlotUpdate.slotPos,
                inventorySlotUpdate.itemId,
                inventorySlotUpdate.itemCount
            );
        }

        InventoryUI* inventoryUI = M_UIMANAGER->GetInventoryUI();
        if (inventoryUI == nullptr)
        {
            throw std::runtime_error("inventoryUI is nullptr");
        }

        inventoryUI->UpdateInventoryByType();
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

void InventoryPacketHandler::SendMoveItem(int inventoryType, int fromSlotPos, int toSlotPos)
{
    std::vector<std::string> data;

    data.push_back(std::to_string(inventoryType));
    data.push_back(std::to_string(fromSlotPos));
    data.push_back(std::to_string(toSlotPos));

    stb::NetworkManager::getInstance()->SendPacket(PKT_INVENTORY_ITEM_MOVE, data);
    OutputDebugStringA("[PKT_INVENTORY_ITEM_MOVE 전송 완료]\n\n");
}

