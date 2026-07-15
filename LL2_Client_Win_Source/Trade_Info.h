#pragma once
#include "CommonInclude.h"
#include "Inventory_Info.h"

//교환 슬롯 하나 (아이템 정보 + 슬롯 위치)
struct TradeSlotInfo
{
	std::string itemId;
	int itemCount = 0;
	int tradeSlotPos = 0;
	int invenSlotPos = 0;
};

//교환 세션 전체 상태
struct TradeSessionInfo
{
	std::string myPlayerId		= 0;
	std::string targetPlayerId	= 0;

	std::vector<TradeSlotInfo> myItems; //내가 올린 아이템
	std::vector<TradeSlotInfo> targetItems; //상대가 올린 아이템

	bool myConfirmed = false;
	bool targetConfirmed = false;
};

//교환 신청 수신 시 팝업에 쓸 정보
struct TradeRequestInfo
{
	std::string requesterId;
	std::string requesterName;
};

//교환 완료 결과
struct TradeCompleteResult
{
	int result = 0; // 0 = 실패, 1 = 성공
	int errcode = 0;
	//완료 후 인벤토리는 PKT_INVENTORY_ITEM_INFO로 별도 갱신됨
};