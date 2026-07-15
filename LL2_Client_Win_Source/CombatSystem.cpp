#include "CombatSystem.h"
#include "..\\LL2_Client_Win_lib\\stbPlayer.h"
#include "CombatPacketHandler.h"
#include "stbMath.h"



CombatSystem::CombatSystem() : m_player(nullptr)
{

}

bool CombatSystem::TryAttack(int skillId)
{
    if (!CanAttack(skillId))
        return false;

    // 플레이어 공격 방향 나중에 추가
    int dir = 1;
 
    stb::FacingDirection direction = m_player->GetFacing();

    if (direction == stb::FacingDirection::Left)
        dir = -1;
    else
        dir = 1;

    // 서버에 공격 패킷 보내기
    CombatPacketHandler::SendUseSkill(skillId, dir);
    return true;
}

bool CombatSystem::TryBasicAttack()
{
    if (m_player == nullptr)
    {
        DebugMsg = "m_player is nullptr \n";
        OutputDebugStringA(DebugMsg.c_str());
        return false;
        
    }
      
    if (!CanBasicAttack())
    {
        DebugMsg = "Cant Basic Attack\n";
        OutputDebugStringA(DebugMsg.c_str());
        return false;
    }
       
    AttackDirection dir =
        (m_player->GetFacing() == stb::FacingDirection::Left) ? AttackDirection::Left : AttackDirection::Right;

    // 서버에 공격 패킷 보내기
    CombatPacketHandler::SendBasicAttack(static_cast<int>(dir));
    m_player->SetState(PlayerState::Attack);
   
    return true;
}

bool CombatSystem::CanAttack(int skillId)
{
  
    if (m_player == nullptr)
    {
        DebugMsg = "m_player is nullptr \n";
        OutputDebugStringA(DebugMsg.c_str());
        return false;
    }
        
    if (m_player->IsDead())
    {
        DebugMsg = "player is Dead \n";
        OutputDebugStringA(DebugMsg.c_str());
        return false;
    }
       

    if (m_player->IsAttacking())
    {
        DebugMsg = "player is Attacking \n";
        OutputDebugStringA(DebugMsg.c_str());
        return false;
    }
       

    // 스킬 쿨타임 여부 확인 추가 필요
    return true;
}

bool CombatSystem::CanBasicAttack()
{
    if (m_player == nullptr)
        return false;

    PlayerState state = m_player->GetState();

    if (state == PlayerState::Dead)
        return false;

    if (state == PlayerState::Attack)
        return false;

    // IDLE, MOVE는 공격 가능
    return true;
}


