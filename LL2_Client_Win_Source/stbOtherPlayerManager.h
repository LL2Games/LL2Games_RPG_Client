#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "stbOtherPlayer.h"
#include "PacketData.h"
#include "CombatSystem_Info.h"

namespace stb
{
    class Scene;
    // 다른 플레이어들을 관리하는 매니저
    class OtherPlayerManager : public SingletonBase<OtherPlayerManager>
    {
    public:
        OtherPlayerManager();
        ~OtherPlayerManager();

        bool HandleMovePacket(OtherPlayerMove& otherPlayerMove);
        bool HandleAttackPacket(OtherPlayerAttack& otherPlayerAttack);

        // 다른 플레이어 추가 또는 업데이트
        bool UpdatePlayer(const std::string& charId, float x, float y);

        bool AddPlayer(OtherPlayerInfo& playerInfo);

        // 다른 플레이어 제거
        void RemovePlayer(const std::string& charId);

        // 모든 플레이어 제거
        void Clear();

        // 플레이어 맵 접근
        std::map<std::string, OtherPlayer*>& GetPlayers() { return m_players; }

        void RemoveAllPlayersFromScene(Scene* scene);
    private:
        std::map<std::string, OtherPlayer*> m_players;
    };
}
