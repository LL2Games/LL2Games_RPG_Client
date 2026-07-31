#include "CombatSystem.h"
#include "..\\LL2_Client_Win_lib\\stbPlayer.h"
#include "CombatPacketHandler.h"
#include "stbMath.h"
#include "SkillDataManager.h"

#define M_SKILLDATA_MANAGER stb::SingletonBase<SkillDataManager>::getInstance()


CombatSystem::CombatSystem() : m_player(nullptr)
{

}

bool CombatSystem::TrySkillAttack(int skillId)
{
    if (!CanUseSkill(skillId))
        return false;

    // 플레이어 공격 방향 나중에 추가
    int dir = 1;
 
    stb::FacingDirection direction = m_player->GetFacing();

    if (direction == stb::FacingDirection::Left)
        dir = -1;
    else
        dir = 1;

    // 서버에 공격 패킷 보내기 //gunoo22 260729 스킬 사용부분 확인
    CombatPacketHandler::SendUseSkill(skillId, dir);
    return true;
}

bool CombatSystem::TryBasicAttack()
{
    if (m_player == nullptr)
    {
        m_debugMsg = "m_player is nullptr \n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
        
    }
      
    if (!CanBasicAttack())
    {
        m_debugMsg = "Cant Basic Attack\n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
    }
       
    AttackDirection dir =
        (m_player->GetFacing() == stb::FacingDirection::Left) ? AttackDirection::Left : AttackDirection::Right;

    // 서버에 공격 패킷 보내기
    CombatPacketHandler::SendBasicAttack(static_cast<int>(dir));
    m_player->SetState(PlayerState::Attack);
   
    return true;
}

bool CombatSystem::CanUseSkill(int skillId)
{
  
    if (m_player == nullptr)
    {
        m_debugMsg = "m_player is nullptr \n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
    }
        
    if (m_player->IsDead())
    {
        m_debugMsg = "player is Dead \n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
    }
       

    if (m_player->IsAttacking())
    {
        m_debugMsg = "player is Attacking \n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
    }
    
    const SkillData* skill = M_SKILLDATA_MANAGER->FindItemData(skillId);
    if (skill == nullptr)
    {
        m_debugMsg = std::to_string(skillId) + " skill is nullptr \n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
    }

    //마나 검사
    const int curMp = m_player->GetStat()->GetCurMp();
    if (curMp < skill->mp_cost)
    {
        m_debugMsg = "플레이어의 마나가 부족합니다. \n";
        OutputDebugStringA(m_debugMsg.c_str());
        return false;
    }

    //TODO 마나는 여기서 줄고있는데 추후에는 실제 사용 이후에 마나를 줄여야함.
    //m_player->GetStat()->SetCurMp(std::max(curMp - skill->mp_cost, 0));
    
    //쿨타임 검사
    /*const int64_t now = NowMs();

    auto coolit = skillCooldownEndMs.find(skillDef->skill_id);
    if (coolit != skillCooldownEndMs.end() && now <= coolit->second)
    {
        K_LOG_TRACE("아직 쿨타임입니다.\n");
        return false;
    }*/

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


