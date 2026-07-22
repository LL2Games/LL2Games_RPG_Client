#include "MonsterManager.h"

void MonsterManager::Init()
{
   
}

void MonsterManager::Update(float deltaTime)
{
    for (auto& monster : m_monsters)
    {
        if (!monster.second->IsDead())
        {
            monster.second->Update(deltaTime);
        }
    }
}

void MonsterManager::Render(stbD2DRenderer& renderer)
{
    for (auto& monster : m_monsters)
    {
        if (!monster.second->IsDead()) 
        {
            monster.second->Render(renderer);
        }
    }
}

void MonsterManager::SpawnMonster(const MonsterSpawnInfo& info)
{
    auto it = m_monsters.find(info.instanceId);

    if (it != m_monsters.end())
    {
        it->second->ResetFromSpawnInfo(info);
        return;
    }

    auto monster = std::make_unique<Monster>();
    monster->Initialize();
    monster->InitFromSpawn(info);

    std::string DebugMsg = "Monster Spawn \n";

    OutputDebugStringA(DebugMsg.c_str());

    m_monsters.emplace(info.instanceId, std::move(monster));
}

void MonsterManager::RemoveMonster(int /*instanceId*/)
{

}

void MonsterManager::ApplyServerUpdate(const MonsterUpdateInfo& info)
{
    auto it = m_monsters.find(info.instanceId);

    if (it == m_monsters.end())
        return;

    Monster* monster = it->second.get();
    if (monster == nullptr)
        return;

    // 이미 사망 애니메이션까지 끝난 몬스터면 이후 이동 갱신은 무시
    if (monster->IsDead() && info.state != MonsterState::E_Die)
        return;

    monster->ApplyServerUpdate(info);
}

void MonsterManager::ApplyAttackResult(const AttackResult& result)
{
    auto it = m_monsters.find(result.monster_instance_id);
    if (it == m_monsters.end())
        return;

    Monster* monster = it->second.get();
    if (monster == nullptr)
        return;

    monster->ApplyAttackResult(result);
}

void MonsterManager::RespawnMonster(const MonsterUpdateInfo& info)
{
    auto it = m_monsters.find(info.instanceId);

    if (it != m_monsters.end())
    {
        // 이미 있으면 재활성화(부활) 처리
        it->second->RespawnFromServer(info);
        OutputDebugStringA("Monster Respawn (existing)\n");
        return;
    }

 
    MonsterSpawnInfo spawn{};
    spawn.instanceId = info.instanceId;
    spawn.monsterId = info.monsterId;
    spawn.pos = info.pos;
    spawn.dir = info.dir;
    spawn.moveSpeed = 0;
    spawn.curHp = info.curHp;
    spawn.maxHp = info.maxHp;
    spawn.state = info.state;

 
    SpawnMonster(spawn);

    OutputDebugStringA("Monster Respawn (created)\n");
}


