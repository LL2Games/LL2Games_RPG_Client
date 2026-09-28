#include "CombatPacketHandler.h"
#include "PacketParser.h"
#include "stbNetworkManager.h"
#include "CombatSystem_Info.h"
#include "MonsterManager.h"
#include "PlayerManager.h"
#include "stbOtherPlayerManager.h"
#include "playerInfo.h"
#include "SkillDataManager.h"
#include "Monster.h"
#include "stbTransform.h"
#include "SkillEffectManager.h"

#define M_SKILLDATA_MANAGER stb::SingletonBase<SkillDataManager>::getInstance()
#define M_SKILLEFFECTMANAGER stb::SingletonBase<SkillEffectManager>::getInstance()
#define M_MONSTERMANAGER stb::SingletonBase<MonsterManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_OTHERPLAYERMANAGER stb::SingletonBase<stb::OtherPlayerManager>::getInstance()


void CombatPacketHandler::HandlerMosterDamage(const ParsedPacket& /*pkt*/)
{
}

void CombatPacketHandler::HandleAttackResult(const ParsedPacket& pkt)
{
   
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;

        int monster_size = 0;
        int isDead = 0;

        AttackResult attackResult{};


        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monster_size, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, attackResult.playerId, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, attackResult.skillId, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        for (size_t i = 0; i < monster_size; i++)
        {
          
            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, attackResult.monster_instance_id, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, attackResult.damage, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, attackResult.cur_hp, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, attackResult.max_hp, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(data, payloadSize, offset, isDead, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            attackResult.isDead = (isDead != 0);


            M_MONSTERMANAGER->ApplyAttackResult(attackResult);

            Monster* monster = M_MONSTERMANAGER->FindMonster(attackResult.monster_instance_id);
            if (monster != nullptr)
            {
                stb::Transform* tr = monster->GetComponent<stb::Transform>();
                if (tr != nullptr)
                {
                    const SkillData* skillData = M_SKILLDATA_MANAGER->FindItemData(attackResult.skillId);

                    if (skillData != nullptr)
                    {
                        M_SKILLEFFECTMANAGER->PlayHit(skillData->skillVfx_name,tr->GetPosition());
                    }
                }
            }

        }
        M_PLAYERMANAGER->PlayAttackAnimation(attackResult.playerId, attackResult.skillId);
    }
    catch (std::exception& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러입니다.\n\n");
    }

}

//스킬 사용 결과
void CombatPacketHandler::HandleSkillAttackResult(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string errMsg;

        std::string status;
        int skill_id = 0;
        int currentMp = 0;


        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (status == "nok")
        {
            if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, errMsg, errMsg))
               throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, skill_id, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, currentMp, errMsg))
        {
            throw std::runtime_error(errMsg);
        }



        auto *player = M_PLAYERMANAGER->GetLocalPlayer();
        if (player == nullptr)
        {
            OutputDebugStringA("player 객체를 찾지못했습니다.\n\n");
            return;
        }
        const auto *skill = M_SKILLDATA_MANAGER->FindItemData(skill_id);
        if (skill == nullptr)
        {
            OutputDebugStringA("Skill id에 맞는 json data를 찾지못했습니다.\n\n");
            return;
        }

        //스킬사용 성공시 현재 Mp를 스킬 mp cost에 맞게 줄임
        //const int curMp = player->GetStat()->GetCurMp();
        //player->GetStat()->SetCurMp(std::max(curMp - skill->mp_cost, 0));
        player->GetStat()->SetCurMp(currentMp);
        
    }
    catch (std::exception& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러입니다.\n\n");
    }
}

void CombatPacketHandler::HandleOtherPlayerAttack(const ParsedPacket& pkt)
{
    /*
    payload.push_back(std::to_string(attacker->GetId()));
    payload.push_back(std::to_string(skillId));
    payload.push_back(std::to_string(attackDir));
    payload.push_back(std::to_string(3));
    */
    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        int state = 0;
        std::string errMsg;
        OutputDebugStringA("[PKT_OTHER_PLAYER_ATTACK received]\n");
        OtherPlayerAttack otherPlayerAttack{};

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, otherPlayerAttack.playerId, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, otherPlayerAttack.skillId, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, otherPlayerAttack.attackDir, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, state, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        otherPlayerAttack.state = PlayerTypeUtil::IntToState(state);

        M_OTHERPLAYERMANAGER->HandleAttackPacket(otherPlayerAttack);

    }
    catch (std::exception& e)
    {   
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러입니다.\n\n");
    }
}

void CombatPacketHandler::SendBasicAttack(int dir)
{
    std::vector<std::string> data;

    data.push_back(std::to_string(dir));

    stb::NetworkManager::getInstance()->SendPacket(PKT_PLAYER_BASIC_ATTACK, data);
    OutputDebugStringA("[PKT_PLAYER_BASIC_ATTACK 전송 완료]\n\n");
}

//gunoo22 260729 스킬 사용부분 확인
void CombatPacketHandler::SendUseSkill(int skillId, int dir)
{
    std::vector<std::string> data;

    data.push_back(std::to_string(skillId));
    data.push_back(std::to_string(dir));

    //std::string DebugMsg = "Use_Count :" + std::to_string(inventoryitemInfo->useCount) + "\n";

    //OutputDebugStringA(DebugMsg.c_str());

    stb::NetworkManager::getInstance()->SendPacket(PKT_PLAYER_SKILL_ATTACK, data);
    OutputDebugStringA("[PKT_PLAYER_SKILL_ATTACK 전송 완료]\n\n");

}


