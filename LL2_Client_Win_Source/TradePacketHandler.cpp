#include "TradePacketHandler.h"
#include "PacketParser.h"
#include "stbNetworkManager.h"
#include "TradeManager.h"
#include "UIManager.h"
#include "InventoryUI.h"

#define M_NETWORK stb::SingletonBase<stb::NetworkManager>::getInstance()
#define M_TRADEMGR stb::SingletonBase<TradeManager>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()

// ── 송신 ─────────────────────────────────────────────────────────────

//교환 신청
void TradePacketHandler::SendTradeRequest(std::string targetPlayerId)
{
	//PacketParser::MakeBody는 문자열 벡터를 받아 length-prefix인코딩
	std::vector<std::string> datas = { targetPlayerId };
	M_NETWORK->SendPacket(PKT_TRADE_REQUEST, datas);

	// TODO: UIManager에 TradeUI 열기
	// UIManager::getInstance()->OpenTradeUI();
}

//교환 수락
void TradePacketHandler::SendTradeAccept(const std::string& requesterId)
{
	M_NETWORK->SendPacket(PKT_TRADE_ACCEPT, {requesterId});
}

//교환 거절
void TradePacketHandler::SendTradeDecline()
{
	//거절은 보낼 필요 없을듯 -> 메이플에 거절했다고 상대에게 알림 안감.
	//M_NETWORK->SendPacket(PKT_TRADE_DECLINE, {});
}

//아이템 추가
void TradePacketHandler::SendTradeAddItem(const TradeSlotInfo& item)
{
	std::vector<std::string> datas = {
		item.itemId,
		std::to_string(item.itemCount),
		std::to_string(item.tradeSlotPos),
		std::to_string(item.invenSlotPos)
	};

	M_NETWORK->SendPacket(PKT_TRADE_ADD_ITEM, datas);
}

//교환 취소
void TradePacketHandler::SendTradeCancel(const std::string& targetName)
{
	M_NETWORK->SendPacket(PKT_TRADE_CANCEL, { targetName });
}

//교환 준비
void TradePacketHandler::SendTradeReady(const std::string& targetId, const std::vector<std::string> &items)
{
	std::vector<std::string> datas;

	datas.push_back(targetId);
	datas.insert(datas.end(), items.begin(), items.end());

	M_NETWORK->SendPacket(PKT_TRADE_READY, datas);
}

// ── 수신 ─────────────────────────────────────────────────────────────

// S→C: 교환 신청이 들어왔을 때
//payload: [requesterId(string)][requesterName(string)]
void TradePacketHandler::HandleTradeRequest(const ParsedPacket& pkt)
{
	size_t offset = 0;
	const char* data = pkt.payload.c_str();
	size_t payloadSize = pkt.payload.size();
	std::string errMsg;

	TradeRequestInfo info;
	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, info.requesterId, errMsg))
		return;
		
	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, info.requesterName, errMsg))
		return;

	//TODO: UIManager에 교환 신청 팝업 표시
	UIManager::getInstance()->ShowTradeRequestPopUp(info);

}

//교환 시작
void TradePacketHandler::HandleTradeStart(const ParsedPacket& pkt)
{
	size_t offset = 0;
	const char* data = pkt.payload.c_str();
	size_t payloadSize = pkt.payload.size();
	std::string targetId, targetName, errMsg;

	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, targetId, errMsg))
		return;

	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, targetName, errMsg))
		return;

	//TODO: UIManager에 교환 신청 팝업 표시
	UIManager::getInstance()->OpenTradeUI(targetId, targetName);
}

//상대 교환 준비
void TradePacketHandler::HandleTradeReady(const ParsedPacket& pkt)
{
	size_t offset = 0;
	const char* data = pkt.payload.c_str();
	size_t payloadSize = pkt.payload.size();
	std::string targetId, errMsg;

	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, targetId, errMsg))
		return;

	if (targetId == "wait" || targetId == "nok")
		return;

	//TODO: UIManager에 교환 신청 팝업 표시
	UIManager::getInstance()->TradeReadyTarget();
}

//교환 완료
void TradePacketHandler::HandleTradeComplete(const ParsedPacket& pkt)
{
	size_t offset = 0;
	const char* data = pkt.payload.c_str();
	size_t payloadSize = pkt.payload.size();
	std::string status, errMsg;

	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, errMsg))
		return;

	if (status == "ok")
	{
		std::vector<TradeSlotInfo> mySlotInfos, targetSlotInfos;
		//내 슬롯 아이템
		while (1)
		{
			TradeSlotInfo slotInfo;
			std::string item_id, item_amount, item_slot_index;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_id, errMsg))
				break;
			if (item_id == "$")
				break;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_amount, errMsg))
				break;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_slot_index, errMsg))
				break;

			slotInfo.itemId = item_id;
			slotInfo.itemCount = std::stoi(item_amount);
			slotInfo.invenSlotPos = std::stoi(item_slot_index);
			mySlotInfos.push_back(slotInfo);
		}

		//상대 슬롯 아이템
		while (1)
		{
			TradeSlotInfo slotInfo;
			std::string item_id, item_amount, item_slot_index;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_id, errMsg))
				break;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_amount, errMsg))
				break;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_slot_index, errMsg))
				break;

			slotInfo.itemId = item_id;
			slotInfo.itemCount = std::stoi(item_amount);
			slotInfo.invenSlotPos = std::stoi(item_slot_index);
			targetSlotInfos.push_back(slotInfo);
		}

		UIManager::getInstance()->ShowSuccessPopUp(mySlotInfos, targetSlotInfos);
	}
}

//교환 취소
void TradePacketHandler::HandleTradeCancel(const ParsedPacket& pkt)
{
	size_t offset = 0;
	const char* data = pkt.payload.c_str();
	size_t payloadSize = pkt.payload.size();
	std::string status, targetName, errMsg;

	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, errMsg))
		return;

	if (status == "nok")
	{
		if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, errMsg, errMsg))
			return;
		OutputDebugStringA(errMsg.c_str());
		return;
	}

	UIManager::getInstance()->ShowCancelPopUp(); //상대가 교환 취소했다는 팝업 
}

//아이템 업로드
void TradePacketHandler::HandleTradeAddItem(const ParsedPacket& pkt)
{
	size_t offset = 0;
	const char* data = pkt.payload.c_str();
	size_t payloadSize = pkt.payload.size();
	std::string status, item_id, item_amount, item_slot_index, errMsg;

	if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, errMsg))
		return;

	if (status == "nok")
	{
		if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, errMsg, errMsg))
			return;
		OutputDebugStringA(errMsg.c_str());
		return;
	}
	else if (status == "ok")
	{
		OutputDebugStringA("success Trade Add Item");
		return;
	}
	else //상대방의 Add Item 수신
	{
		item_id = status;
		if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_amount, errMsg))
			return;

		if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, item_slot_index, errMsg))
			return;
	}

	//TODO
	//상대방 아이템 업로드 UI 업데이트
	TradeSlotInfo tradeSlotInfo;
	tradeSlotInfo.itemId = item_id;
	tradeSlotInfo.itemCount = std::stoi(item_amount);
	tradeSlotInfo.tradeSlotPos = std::stoi(item_slot_index);
	UIManager::getInstance()->OnTradeAddItem(tradeSlotInfo);
}