#include "ShopPacketHandler.h"

#include "PacketParser.h"
#include "ShopManager.h"
#include "PlayerManager.h"
#include "stbNetworkManager.h"
#include "InventoryManager.h"
#include "UIManager.h"

#include <stdexcept>
#include <unordered_set>
#include <utility>

void ShopPacketHandler::HandleOpen(const ParsedPacket& packet)
{
    try
    {
        const char* data = packet.payload.data();
        const size_t payloadSize = packet.payload.size();
        size_t offset = 0;

        std::string error;
        std::string status;

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, error))
        {
            throw std::runtime_error(error);
        }

        if (status != "ok")
            throw std::runtime_error("unexpected shop open status");

        ShopOpenResult result{};
        int allowSell = 0;
        int productCount = 0;

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, result.mapId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.spawnId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.npcId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.shopId, error) ||
            !PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, result.name, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, allowSell, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, productCount, error))
        {
            throw std::runtime_error(error);
        }

        if (result.mapId <= 0 || result.spawnId <= 0 || result.npcId <= 0 || result.shopId <= 0 ||
            (allowSell != 0 && allowSell != 1) || productCount < 0 ||  productCount > 1000)
        {
            throw std::runtime_error("invalid shop open data");
        }

        result.allowSell = allowSell == 1;
        result.products.reserve(productCount);

        std::unordered_set<int> productIds;

        for (int i = 0; i < productCount; ++i)
        {
            ShopProduct product{};

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, product.productId, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, product.itemId, error) ||
                !PacketParser::ParseNextInt64Field(data, payloadSize, offset, product.price, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset,product.maxPerPurchase, error))
            {
                throw std::runtime_error(error);
            }

            if (product.productId <= 0 || product.itemId <= 0 || product.price <= 0 || product.maxPerPurchase <= 0 || !productIds.insert(product.productId).second)
            {
                throw std::runtime_error("invalid shop product");
            }

            result.products.push_back(product);
        }

        if (offset != payloadSize)
            throw std::runtime_error("unexpected payload fields");

        auto* player = PlayerManager::getInstance()->GetLocalPlayer();

        // 이전 맵에서 도착한 상점 정보는 적용하지 않는다.
        if (player == nullptr || player->IsDead() || player->GetPlayerLocation()->mapId != result.mapId)
        {
            return;
        }

        ShopManager::getInstance()->Open(std::move(result));
        UIManager::getInstance()->ResetShopSelection();
        OutputDebugStringA("[Shop] product list received\n");
    }
    catch (const std::exception& exception)
    {
        OutputDebugStringA((std::string("[Shop open] ") + exception.what() + "\n").c_str());
    }
}

void ShopPacketHandler::HandleBuyResult(const ParsedPacket& packet)
{
    try
    {
        const char* data = packet.payload.data();
        const size_t payloadSize = packet.payload.size();
        size_t offset = 0;

        std::string error;
        std::string status;

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, error))
        {
            throw std::runtime_error(error);
        }

        if (status == "nok")
        {
            std::string reason;

            if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, reason, error))
            {
                throw std::runtime_error(error);
            }

            if (offset != payloadSize)
                throw std::runtime_error("unexpected payload fields");

            OutputDebugStringA(("[Shop buy failed] " + reason + "\n").c_str());
            return;
        }

        if (status != "ok")
            throw std::runtime_error("invalid shop buy status");

        ShopBuyResult result{};
        int slotCount = 0;

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, result.mapId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.spawnId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.shopId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.productId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.itemId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.count, error) ||
            !PacketParser::ParseNextInt64Field(data, payloadSize, offset, result.totalPrice, error) ||
            !PacketParser::ParseNextInt64Field(data, payloadSize, offset, result.gold, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, slotCount, error))
        {
            throw std::runtime_error(error);
        }

        if (result.mapId <= 0 ||
            result.spawnId <= 0 ||
            result.shopId <= 0 ||
            result.productId <= 0 ||
            result.itemId <= 0 ||
            result.count <= 0 ||
            result.totalPrice <= 0 ||
            result.gold < 0 ||
            slotCount <= 0 ||
            slotCount > 1000)
        {
            throw std::runtime_error("invalid shop buy result");
        }

        result.updatedSlots.reserve(slotCount);

        for (int i = 0; i < slotCount; ++i)
        {
            ShopSlotUpdate slot{};

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, slot.inventoryType, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.slotPos, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.itemId, error) ||
                !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.itemCount, error))
            {
                throw std::runtime_error(error);
            }

            if (slot.inventoryType < 0 ||
                slot.slotPos < 0 ||
                slot.itemId != result.itemId ||
                slot.itemCount <= 0)
            {
                throw std::runtime_error("invalid shop slot update");
            }

            result.updatedSlots.push_back(slot);
        }

        if (offset != payloadSize)
            throw std::runtime_error("unexpected payload fields");

        auto* player = PlayerManager::getInstance()->GetLocalPlayer();

        if (player == nullptr)
            throw std::runtime_error("local player is nullptr");

        auto* inventoryManager = player->GetInvenManager();

        if (inventoryManager == nullptr)
            throw std::runtime_error("inventory manager is nullptr");

        // 먼저 복사본에 전체 변경을 적용한다.
        // 잘못된 슬롯이 있으면 실제 인벤토리는 변경하지 않는다.
        const int inventoryType = result.updatedSlots.front().inventoryType;

        auto* inventory = inventoryManager->GetInventory(inventoryType);

        if (inventory == nullptr)
            throw std::runtime_error("inventory not found");

        Inventory updatedInventory = *inventory;

        for (const auto& slot : result.updatedSlots)
        {
            if (slot.inventoryType != inventoryType || !updatedInventory.ApplyServerSlot( slot.slotPos, slot.itemId, slot.itemCount))
            {
                throw std::runtime_error("cannot apply shop slot");
            }
        }

        *inventory = std::move(updatedInventory);
        player->SetGold(result.gold);

        UIManager::getInstance()->RefreshInventoryUI();

        OutputDebugStringA("[Shop] purchase completed\n");
    }
    catch (const std::exception& exception)
    {
        OutputDebugStringA((std::string("[Shop buy] ") + exception.what() + "\n").c_str());
    }
}

void ShopPacketHandler::HandleSellResult(const ParsedPacket& packet)
{
    try
    {
        const char* data = packet.payload.data();
        const size_t payloadSize = packet.payload.size();
        size_t offset = 0;

        std::string error;
        std::string status;

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, error))
        {
            throw std::runtime_error(error);
        }

        if (status == "nok")
        {
            std::string reason;

            if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, reason, error))
            {
                throw std::runtime_error(error);
            }

            if (offset != payloadSize)
                throw std::runtime_error("unexpected payload fields");

            OutputDebugStringA(("[Shop sell failed] " + reason + "\n").c_str());
            return;
        }

        if (status != "ok")
            throw std::runtime_error("invalid shop sell status");

        ShopSellResult result{};
        auto& slot = result.updatedSlot;

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, result.mapId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.spawnId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.shopId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.itemId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, result.count, error) ||
            !PacketParser::ParseNextInt64Field(data, payloadSize, offset, result.totalPrice, error) ||
            !PacketParser::ParseNextInt64Field(data, payloadSize, offset, result.gold, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.inventoryType, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.slotPos, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.itemId, error) ||
            !PacketParser::ParseNextIntField(data, payloadSize, offset, slot.itemCount, error))
        {
            throw std::runtime_error(error);
        }

        if (offset != payloadSize)
            throw std::runtime_error("unexpected payload fields");

        if (result.mapId <= 0 ||
            result.spawnId <= 0 ||
            result.shopId <= 0 ||
            result.itemId <= 0 ||
            result.count <= 0 ||
            result.totalPrice <= 0 ||
            result.gold < 0 ||
            slot.inventoryType < 0 ||
            slot.slotPos < 0 ||
            slot.itemId < 0 ||
            slot.itemCount < 0 ||
            ((slot.itemId == 0) != (slot.itemCount == 0)) ||
            (slot.itemId != 0 && slot.itemId != result.itemId))
        {
            throw std::runtime_error("invalid shop sell result");
        }

        auto* player = PlayerManager::getInstance()->GetLocalPlayer();

        if (player == nullptr)
            throw std::runtime_error("local player is nullptr");

        auto* inventoryManager = player->GetInvenManager();

        if (inventoryManager == nullptr)
            throw std::runtime_error("inventory manager is nullptr");

        auto* inventory = inventoryManager->GetInventory(slot.inventoryType);

        if (inventory == nullptr)
            throw std::runtime_error("inventory not found");

        Inventory updatedInventory = *inventory;

        if (!updatedInventory.ApplyServerSlot(slot.slotPos, slot.itemId, slot.itemCount))
        {
            throw std::runtime_error("cannot apply shop slot");
        }

        *inventory = std::move(updatedInventory);
        player->SetGold(result.gold);

        UIManager::getInstance()->RefreshInventoryUI();

        OutputDebugStringA("[Shop] sale completed\n");
    }
    catch (const std::exception& exception)
    {
        OutputDebugStringA((std::string("[Shop sell] ") + exception.what() + "\n").c_str());
    }
}

void ShopPacketHandler::SendBuy(int productId, int count)
{
    OutputDebugStringA("[Shop buy] SendBuy called\n");

    auto* network = stb::NetworkManager::getInstance();
    auto* player = PlayerManager::getInstance()->GetLocalPlayer();
    const auto* shop = ShopManager::getInstance()->GetCurrentShop();

    if (!network->IsConnected() || player == nullptr || player->IsDead() || shop == nullptr || player->GetPlayerLocation()->mapId != shop->mapId)
    {
        return;
    }

    const ShopProduct* selected = nullptr;

    for (const auto& product : shop->products)
    {
        if (product.productId == productId)
        {
            selected = &product;
            break;
        }
    }

    if (selected == nullptr || count <= 0 || count > selected->maxPerPurchase)
    {
        return;
    }

    OutputDebugStringA("[Shop buy] sending request\n");

    network->SendPacket(PKT_SHOP_BUY,
        {
            std::to_string(shop->mapId),
            std::to_string(shop->spawnId),
            std::to_string(productId),
            std::to_string(count)
        });
}

void ShopPacketHandler::SendSell(int inventoryType, int slotPos, int count)
{
    auto* network = stb::NetworkManager::getInstance();
    auto* player = PlayerManager::getInstance()->GetLocalPlayer();
    const auto* shop = ShopManager::getInstance()->GetCurrentShop();

    if (!network->IsConnected() ||
        player == nullptr ||
        player->IsDead() ||
        shop == nullptr ||
        !shop->allowSell ||
        player->GetPlayerLocation()->mapId != shop->mapId ||
        slotPos < 0 ||
        count <= 0)
    {
        return;
    }

    auto* inventoryManager = player->GetInvenManager();

    if (inventoryManager == nullptr)
        return;

    const auto* item = inventoryManager->FindSlot(inventoryType, slotPos);

    if (item == nullptr || item->itemId <= 0 || count > item->itemCount)
    {
        return;
    }

    network->SendPacket(PKT_SHOP_SELL,
        {
            std::to_string(shop->mapId),
            std::to_string(shop->spawnId),
            std::to_string(inventoryType),
            std::to_string(slotPos),
            std::to_string(count)
        });
}
