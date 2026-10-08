#pragma once

#include "Packet.h"

class ShopPacketHandler
{
public:
    static void HandleOpen(const ParsedPacket& packet);
    static void HandleBuyResult(const ParsedPacket& packet);
    static void HandleSellResult(const ParsedPacket& packet);
    static void SendBuy(int productId, int count);
    static void SendSell(int inventoryType, int slotPos, int count);

};
