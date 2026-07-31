#pragma once
#include "CommonInclude.h"
#include "..\\LL2_Client_Win_lib\\\stbPlayer.h"
#include "stbSingletonBase.h"
#include "stbmath.h"
#include "stbEnums.h"

class PlayerManager : public stb::SingletonBase<PlayerManager>
{
public:


public:
	stb::Player* GetLocalPlayer() { return m_localPlayer; }

	stb::Player* CreateLocalPlayer(stb::enums::eLayerType layer, const stb::math::Vector2& pos);
	void SetLocalPlayer(stb::Player* player) { m_localPlayer = player; }
	void PlayAttackAnimation(int playerId, int skillId);

	stb::Player* FindPlayer(int playerId);
private:
	stb::Player* m_localPlayer =nullptr;
	std::unordered_map<int, stb::Player*> m_otherPlayers;



};

