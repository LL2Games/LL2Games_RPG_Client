#pragma once
#include "IPacketHandler.h"
#include "Packet.h"
#include "CommonInclude.h"
#include "Trade_Info.h"

class TradePacketHandler : public IPacketHandler
{
public:
	// --- S->C 수신 핸들러 ------------------------------------------------------
	static void HandleTradeRequest(const ParsedPacket& pkt);	//교환신청 받음
	static void HandleTradeStart(const ParsedPacket& pkt);		//교환 시작
	static void HandleTradeReady(const ParsedPacket& pkt);		//상대가 교환 준비
	static void HandleTradeDeclined(const ParsedPacket& pkt);	//상대가 거절
	static void HandleTradeItemUpdate(const ParsedPacket& pkt); //상대 아이템 목록 갱신
	static void HandleTradeComplete(const ParsedPacket& pkt);	//교환완료
	static void HandleTradeCancel(const ParsedPacket& pkt);		//교환 취소
	static void HandleTradeAddItem(const ParsedPacket& pkt);	//아이템 추가


	// --- C->S 송신 함수 ------------------------------------------------------
	static void SendTradeRequest(std::string targetPlayerId);	//교환신청
	static void SendTradeAccept(const std::string &requesterId);								//교환수락
	static void SendTradeDecline();								//교환거절
	static void SendTradeAddItem(const TradeSlotInfo& item);	//아이템 추가
	static void SendTradeReady(const std::string& targetId, const std::vector<std::string>& items = std::vector<std::string>());								//교환준비
	static void SendTradeRemoveItem(int slotPos);				//아이템 제거
	static void SendTradeConfirm();								//교환확인
	static void SendTradeCancel(const std::string &targetName);								//교환취소

};