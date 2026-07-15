#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "Monster.h"
#include "CombatSystem_Info.h"
class stbD2DRenderer;

class MonsterManager : public stb::SingletonBase<MonsterManager>
{
public:
    void Init();
    void Update(float deltaTime);
    void Render(stbD2DRenderer& renderer);


    void SpawnMonster(const MonsterSpawnInfo& info);
    void RemoveMonster(int instanceId);
    void ApplyServerUpdate(const MonsterUpdateInfo& info);
    void ApplyAttackResult(const AttackResult& result);
    void RespawnMonster(const MonsterUpdateInfo& info);
    //void ApplyMonsterDamage(const MonsterHitInfo& info);


private:
    std::unordered_map<int, std::unique_ptr<Monster>> m_monsters;
};

