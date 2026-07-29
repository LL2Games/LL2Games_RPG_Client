#include "PacketManager.h"
#include "Packet.h"
#include "stbNetworkManager.h"
#include "MovePacketHandler.h"
#include "ChannelInitPacketHandler.h"
#include "InventoryPacketHandler.h"
#include "PlayerDataPacketHandler.h"
#include "ItemPacketHandler.h"
#include "MonsterPacketHandler.h"
#include "CombatPacketHandler.h"
#include "QuickSlotPacketHandler.h"
#include "TradePacketHandler.h"
#include "DropItemPacketHandler.h"
#include "OtherPlayerPacketHandler.h"
#include "PortalPacketHandler.h"

bool PacketManager::RegisterAllHandlers()
{
	auto networkManager = stb::NetworkManager::getInstance();

	// 플레이어 접속 핸들러 등록
	networkManager->RegisterHandler(PKT_CHANNEL_AUTH,
		[](const ParsedPacket& pkt)
		{
			ChannelInitPacketHandler::Execute(pkt);
		});

	// 플레이어 기본정보 핸들러 등록
	networkManager->RegisterHandler(PKT_PLAYER_INFO,
		[](const ParsedPacket& pkt)
		{
			PlayerDataPacketHandler::HandleLocalPlayerInfo(pkt);
		});

	// 플레이어 스탯 정보 핸들러 등록
	networkManager->RegisterHandler(PKT_PLAYER_STAT,
		[](const ParsedPacket& pkt)
		{
			PlayerDataPacketHandler::HandleLocalPlayerStat(pkt);
		});

	// 플레이어 온데미지 핸들러 등록
	networkManager->RegisterHandler(PKT_PLAYER_ONDAMAGED,
		[](const ParsedPacket& pkt)
		{
			PlayerDataPacketHandler::HandlePlayerOnDamaged(pkt);
		});

	// 플레이어 인벤토리 핸들러 등록
	networkManager->RegisterHandler(PKT_INVENTORY_META_INFO,
		[](const ParsedPacket& pkt)
		{
			InventoryPacketHandler::HandleInventoryMetaInfo(pkt);
		});

	// 플레이어 인벤토리 핸들러 등록
	networkManager->RegisterHandler(PKT_INVENTORY_ITEM_INFO,
		[](const ParsedPacket& pkt)
		{
			InventoryPacketHandler::HandleInventoryItemInfo(pkt);
		});

	// 플레이어 인벤토리 핸들러 등록
	networkManager->RegisterHandler(PKT_INVENTORY_ITEM_MOVE,
		[](const ParsedPacket& pkt)
		{
			InventoryPacketHandler::HandleInventoryMoveItem(pkt);
		});

	// 플레이어 움직임 핸들러 등록
	networkManager->RegisterHandler(PKT_PLAYER_MOVE,
		[](const ParsedPacket& pkt)
		{
			MovePacketHandler::Execute(pkt);
		});

	// 플레이어 아이템 사용 핸들러 등록
	networkManager->RegisterHandler(PKT_PLAYER_USE_ITEM,
		[](const ParsedPacket& pkt)
		{
			ItemPacketHandler::HandleUseItemResult(pkt);
		});

	// 플레이어 아이템 픽업 핸들러 등록
	networkManager->RegisterHandler(PKT_PLAYER_PICKUP_ITEM,
		[](const ParsedPacket& pkt)
		{
			ItemPacketHandler::HandlePickUpItem(pkt);
		});

	// 다른 플레이어 입장 핸들러 등록
	networkManager->RegisterHandler(PKT_OTHERPLAYER_ENTER,
		[](const ParsedPacket& pkt)
		{
			OtherPlayerPacketHandler::HandleOtherPlayerEnter(pkt);
		});

	// 다른 플레이어 스냅샷 핸들러 등록
	networkManager->RegisterHandler(PKT_OTHERPLAYER_SNAPSHOT,
		[](const ParsedPacket& pkt)
		{
			OtherPlayerPacketHandler::HandleOtherPlayerSnapShot(pkt);
		});

	// 다른 플레이어 공격 핸들러 등록
	networkManager->RegisterHandler(PKT_OTHER_PLAYER_ATTACK,
		[](const ParsedPacket& pkt)
		{
			CombatPacketHandler::HandleOtherPlayerAttack(pkt);
		});

	// 몬스터 스냅샷 핸들러 등록
	networkManager->RegisterHandler(PKT_MONSTER_SNAPSHOT,
		[](const ParsedPacket& pkt)
		{
			MonsterPacketHandler::HandleS2C_SpawnMonster(pkt);
		});

	// 몬스터 업데이트 핸들러 등록
	networkManager->RegisterHandler(PKT_MONSTER_MOVE,
		[](const ParsedPacket& pkt)
		{
			MonsterPacketHandler::HandleS2C_MonsterMove(pkt);
		});

	// 몬스터 데미지 핸들러 등록
	networkManager->RegisterHandler(PKT_MONSTER_ONDAMAGED,
		[](const ParsedPacket& pkt)
		{
			CombatPacketHandler::HandleAttackResult(pkt);
		});

	// 몬스터 리스폰 핸들러 등록
	networkManager->RegisterHandler(PKT_MONSTER_RESPAWN,
		[](const ParsedPacket& pkt)
		{
			MonsterPacketHandler::HandleS2C_RespawnMonster(pkt);
		});

	// 몬스터 투사체 핸들러 등록
	networkManager->RegisterHandler(PKT_PROJECTILE_MOVE,
		[](const ParsedPacket& pkt)
		{
			MonsterPacketHandler::HandleS2C_ProjectileMove(pkt);
		});

	// 퀵슬롯 리스트 핸들러 등록
	networkManager->RegisterHandler(PKT_QUICKSLOT_LIST,
		[](const ParsedPacket& pkt)
		{
			QuickSlotPacketHandler::HandleSlotList(pkt);
		});

	// 퀵슬롯 등록 핸들러 등록
	networkManager->RegisterHandler(PKT_QUICKSLOT_SET,
		[](const ParsedPacket& pkt)
		{
			QuickSlotPacketHandler::HandleSlotSet(pkt);
		});

	// 교환 신청 핸들러 등록
	networkManager->RegisterHandler(PKT_TRADE_REQUEST,
		[](const ParsedPacket& pkt)
		{
			TradePacketHandler::HandleTradeRequest(pkt);
		});

	// 경험치 획득 핸들러 등록
	networkManager->RegisterHandler(PKI_PLAYER_EXP_GAIN,
		[](const ParsedPacket& pkt)
		{
			PlayerDataPacketHandler::HandleLocalPlayerGetExp(pkt);
		});

	// 드롭 아이템 핸들러 등록
	networkManager->RegisterHandler(PKT_DROPITEMS,
		[](const ParsedPacket& pkt)
		{
			DropItemPacketHandler::HandleSpawnDropItem(pkt);
		});

	// 드롭 아이템 삭제 핸들러 등록
	networkManager->RegisterHandler(PKT_REMOVEITEMS,
		[](const ParsedPacket& pkt)
		{
			DropItemPacketHandler::HandleRemoveDropItem(pkt);
		});
	// 교환 시작 핸들러 등록
	networkManager->RegisterHandler(PKT_TRADE_START,
		[](const ParsedPacket& pkt)
		{
			TradePacketHandler::HandleTradeStart(pkt);
		});

	// 교환 취소 핸들러 등록
	networkManager->RegisterHandler(PKT_TRADE_CANCEL,
		[](const ParsedPacket& pkt)
		{
			TradePacketHandler::HandleTradeCancel(pkt);
		});

	// 교환 준비 핸들러 등록
	networkManager->RegisterHandler(PKT_TRADE_READY,
		[](const ParsedPacket& pkt)
		{
			TradePacketHandler::HandleTradeReady(pkt);
		});

	// 교환 완료 핸들러 등록
	networkManager->RegisterHandler(PKT_TRADE_CONFIRM,
		[](const ParsedPacket& pkt)
		{
			TradePacketHandler::HandleTradeComplete(pkt);
		});

	// 교환 아이템 추가 핸들러 등록
	networkManager->RegisterHandler(PKT_TRADE_ADD_ITEM,
		[](const ParsedPacket& pkt)
		{
			TradePacketHandler::HandleTradeAddItem(pkt);
		});

	//맵 이동 핸들러 등록
	networkManager->RegisterHandler(PKT_PORTAL_ENTER,
		[](const ParsedPacket& pkt)
		{
			PortalPacketHandler::HandleMoveMap(pkt);
		});
	
	return true;
}
