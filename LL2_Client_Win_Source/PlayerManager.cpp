#include "PlayerManager.h"
#include "stbObject.h"


stb::Player* PlayerManager::CreateLocalPlayer(stb::enums::eLayerType layer, const stb::math::Vector2& /*pos*/)
{
    stb::Player* player = stb::object::Instantiate<stb::Player>(layer);
    if (player == nullptr)
        return nullptr;

    player->SetLocalPlayer(true);

    m_localPlayer = player;

    player->Initialize();
    return player;
}

void PlayerManager::PlayAttackAnimation(int playerId, int skillId)
{
	// 로컬 플레이어면 플레이어의 상태를 공격중으로 변경하고 애니메이션을 실행

	// 로컬플레이어의 경우 넘긴다.
    if (m_localPlayer == nullptr)
        return;

    if (m_localPlayer->GetPlayerIdentity()->charId == playerId)
        return;

    stb::Player* player = FindPlayer(playerId);
    if (player == nullptr)
        return;

    player->SetState(PlayerState::Attack);
    player->PlayAttackAnimation(skillId);
}

stb::Player* PlayerManager::FindPlayer(int playerId)
{
    auto player = m_otherPlayers.find(playerId);

    if (player == m_otherPlayers.end())
    {
        return nullptr;
    }

    return player->second;
}
