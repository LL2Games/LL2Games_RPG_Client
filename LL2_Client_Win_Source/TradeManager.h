#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "Trade_Info.h"

class TradeManager : public stb::SingletonBase<TradeManager>
{
public:
	//교환 세션 시작/종료
	void StartSession(std::string myId, std::string targetId);
	void EndSession();
	bool IsInTrade() const { return m_inTrade; }

	//아이템 등록/취소
	bool AddMyItem(const TradeSlotInfo& item);
	bool RemoveMyItem(int slotPos);

	//서버에서 받은 상대방 아이템 갱신
	void UpdateTargetItems(const std::vector<TradeSlotInfo>& items);

	//확정 상태
	void SetMyConfirmed(bool confirmed);
	void SetTargetConfirmed(bool confirmed);

	const TradeSessionInfo& GetSession() const { return m_session; }
	TradeSessionInfo& GetSession() { return m_session; }

private:
	bool	m_inTrade = false;
	TradeSessionInfo m_session;
};