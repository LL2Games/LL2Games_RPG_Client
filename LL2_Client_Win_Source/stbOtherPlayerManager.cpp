#include "stbOtherPlayerManager.h"
#include "stbObject.h"
#include "stbTransform.h"
#include "stbAnimator.h"
#include "stbSpriteRenderer.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbEnums.h"
#include "PlayerAnimationManager.h"


#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_PLAYERANIMMNAGER stb::SingletonBase<PlayerAnimationManager>::getInstance()

namespace stb
{
    OtherPlayerManager::OtherPlayerManager()
    {
    }

    OtherPlayerManager::~OtherPlayerManager()
    {
        Clear();
    }

    bool OtherPlayerManager::HandleMovePacket(OtherPlayerMove& otherPlayerMove)
    {
        auto it = m_players.find(otherPlayerMove.playerId);
        if (it != m_players.end() && it->second != nullptr)
        {
            PlayerState effectiveState = otherPlayerMove.state;
            Transform* tr = it->second->GetComponent<Transform>();
            if (tr != nullptr && effectiveState == PlayerState::Idle)
            {
                Vector2 currentPos = tr->GetPosition();
                Vector2 targetPos(otherPlayerMove.xPos, otherPlayerMove.yPos);
                if ((targetPos - currentPos).length() > 0.5f)
                {
                    effectiveState = PlayerState::Walk;
                }
            }

            it->second->SetTargetPosition(otherPlayerMove.xPos, otherPlayerMove.yPos, otherPlayerMove.speed);
            it->second->SetDirection(otherPlayerMove.dir);
            it->second->SetState(effectiveState);
        }
        return true;
        
    }

    bool OtherPlayerManager::HandleAttackPacket(OtherPlayerAttack& otherPlayerAttack)
    {
       
        std::string player_id = std::to_string(otherPlayerAttack.playerId);
        auto it = m_players.find(player_id);
        if (it == m_players.end() || it->second == nullptr)
        {
            OutputDebugStringA("[OtherPlayerAttack] player not found\n");
            return false;
        }

        OtherPlayer* player = it->second;

        player->SetState(PlayerState::Attack);

        return true;
    }

    bool OtherPlayerManager::AddPlayer(OtherPlayerInfo& playerInfo)
    {
        OutputDebugStringA("[OtherPlayer] AddPlayer path\n");
        // 키는 string으로 관리되고 있으므로 변환
        std::string charId = std::to_string(playerInfo.char_id);

        auto it = m_players.find(charId);
        if (it != m_players.end())
        {
            // 이미 존재하면 위치/상태만 업데이트
            OtherPlayer* existing = it->second;
            if (existing)
            {
                existing->UpdatePosition(playerInfo.xPos, playerInfo.yPos);
                existing->SetTargetPosition(playerInfo.xPos, playerInfo.yPos, playerInfo.speed);
                existing->SetDirection(playerInfo.dir);
                existing->SetState(playerInfo.state);
            }
            return true;
        }

        // 새로운 플레이어 생성
        OtherPlayer* player = object::Instantiate<OtherPlayer>(enums::eLayerType::Player, Vector2(playerInfo.xPos, playerInfo.yPos));
        if (player == nullptr)
        {
            OutputDebugStringA("플레이어 생성 실패!\n");
            return false;
        }

        player->SetCharacterId(charId);
        player->SetNickName(playerInfo.name);

        player->Initialize();

        Transform* tr = player->GetComponent<Transform>();
        if (tr != nullptr)
        {
            tr->SetPosition(Vector2(playerInfo.xPos, playerInfo.yPos));
        }

        // 애니메이션 설정: 서버에서 job 정보가 전달되므로 그 값을 사용
        JobType jobType = JobType::None;
        OutputDebugStringA("플레이어 직업 타입 : ");
        OutputDebugStringA(std::to_string(static_cast<int>(jobType)).c_str());
        OutputDebugStringA("\n");
        switch (playerInfo.job)
        {
            case 1: jobType = JobType::Warrior; break;
            case 2: jobType = JobType::Archer;  break;
            case 3: jobType = JobType::Thief;   break;
            case 4: jobType = JobType::Mage;    break;
            default: jobType = JobType::None;   break;
        }
        jobType = JobType::Warrior;
        // 현재 OtherPlayerInfo에 weapon 필드가 없으므로 기본값 사용(필요하면 패킷 구조 변경)
        WeaponType weaponType = WeaponType::One_Hand;

        // PlayerAnimationManager를 통해 애니메이션 셋업 시도
         
        bool setupOk = false;
      
        setupOk = M_PLAYERANIMMNAGER->SetupOtherPlayerAnimations(player, jobType, weaponType);
        

        // 폴백: 기존 하드코드 애니메이션 등록 (리소스가 존재할 때만)
        if (!setupOk)
        {
            Texture* knightTex = M_REMANAGER->Find<Texture>(L"DamonKnight");
            if (knightTex != nullptr)
            {
                // AddComponent 대신 Initialize()에서 컴포넌트를 추가하므로 GetComponent로 확인
                Animator* animator = player->GetComponent<Animator>();
                if (animator == nullptr)
                {
                    animator = player->AddComponent<Animator>();
                }   

                if (animator != nullptr)
                {
                    // 기본 애니메이션명은 OtherPlayer::SetState에서 사용하는 이름과 맞춤
                    animator->CreateAnimation(L"stand", knightTex, Vector2(0.0f, 0.0f), Vector2(67.0f, 81.0f), Vector2::Zero, 1, 1.0f);
                    animator->CreateAnimation(L"walk", knightTex, Vector2(0.0f, 0.0f), Vector2(67.0f, 81.0f), Vector2::Zero, 3, 0.3f);
                    animator->CreateAnimation(L"swingO3", knightTex, Vector2(0.0f, 0.0f), Vector2(67.0f, 81.0f), Vector2::Zero, 3, 0.2f);

                    animator->PlayAnimation(L"stand", true);
                }
            }
        }

     
        // 초기 상태/방향/목표 설정
        player->SetDirection(playerInfo.dir);
        player->SetState(playerInfo.state);
        player->SetTargetPosition(playerInfo.xPos, playerInfo.yPos, playerInfo.speed);

        m_players[charId] = player;

        std::string msg = "다른 플레이어 생성: ID=" + charId + " at (" + std::to_string((int)playerInfo.xPos) + ", " + std::to_string((int)playerInfo.yPos) + ")\n";
        OutputDebugStringA(msg.c_str());

        return true;
    }

    bool OtherPlayerManager::UpdatePlayer(const std::string& charId, float x, float y)
    {
        // 이미 존재하는 플레이어인지 확인
        auto it = m_players.find(charId);
        
        if (it != m_players.end())
        {
            // 기존 플레이어 위치 업데이트
            if (it->second != nullptr)
            {
                it->second->UpdatePosition(x, y);
            }
        }
        return true;
    }

    void OtherPlayerManager::RemovePlayer(const std::string& charId)
    {
        auto it = m_players.find(charId);
        if (it != m_players.end())
        {
            if (it->second != nullptr)
            {
                delete it->second;
            }
            m_players.erase(it);
            
            std::string msg = "다른 플레이어 제거: " + charId + "\n";
            OutputDebugStringA(msg.c_str());
        }

    }

    void OtherPlayerManager::Clear()
    {
        for (auto& pair : m_players)
        {
            if (pair.second != nullptr)
            {
                delete pair.second;
            }
        }
        m_players.clear();
    }

    void OtherPlayerManager::RemoveAllPlayersFromScene(Scene* scene)
    {
        if (scene == nullptr)
            return;
        for (auto& pair : m_players)
        {
            OtherPlayer* player = pair.second;
            if (player != nullptr)
            {
                scene->GetLayer(enums::eLayerType::Player)->RemoveGameObject(player);
                delete player;
            }
        }
        m_players.clear();
    }
}
