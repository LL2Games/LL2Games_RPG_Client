#pragma once

#include "Packet.h"
#include "CommonInclude.h"

class ChatPacketHandler
{
public:
	// --- S->C 수신 핸들러 ------------------------------------------------------
	static void HandleChatInit(const ParsedPacket& pkt);		//채팅 초기화 수신
	static void HandleChat(const ParsedPacket& pkt);			//채팅 수신처리
	 
	//static void HandleTradeRequest(const ParsedPacket& pkt);	//교환신청 받음
	//static void HandleTradeAccepted(const ParsedPacket& pkt);	//상대가 수락
	//static void HandleTradeDeclined(const ParsedPacket& pkt);	//상대가 거절
	//static void HandleTradeItemUpdate(const ParsedPacket& pkt); //상대 아이템 목록 갱신
	//static void HandleTradeComplete(const ParsedPacket& pkt);	//교환완료
	//static void HandleTradeCancel(const ParsedPacket& pkt);		//교환 취소

	// --- C->S 송신 함수 ------------------------------------------------------
	static void SendChatInit(const std::string& charId);	//채팅 초기화
	static void SendChat(const std::string& msg);			//채팅 

	//static void SendTradeRequest(std::string targetPlayerId);	//교환신청
	//static void SendTradeAccept();								//교환수락
	//static void SendTradeDecline();								//교환거절
	//static void SendTradeAddItem(const TradeSlotInfo& item);	//아이템 추가
	//static void SendTradeRemoveItem(int slotPos);				//아이템 제거
	//static void SendTradeConfirm();								//교환확인
	//static void SendTradeCancel();								//교환취소

};
