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

    // 죽는 중에는 일반 이동/상태 패킷만 차단
    if (monster->IsDying())
    {
        if (info.state != MonsterState::E_Die)
            return;
    }

    // 완전히 죽은 객체의 일반 업데이트 차단
    if (monster->IsDead())
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

Monster* MonsterManager::FindMonster(int instanceId)
{
    auto it = m_monsters.find(instanceId);

    if (it == m_monsters.end())
        return nullptr;

    return it->second.get();
}

void MonsterManager::Clear()
{
    m_monsters.clear();

    OutputDebugStringA("[MonsterManager] Clear\n");
}

