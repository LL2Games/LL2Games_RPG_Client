#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "Monster.h"
#include "CombatSystem_Info.h"
#include <vector>
class stbD2DRenderer;

class MonsterManager : public stb::SingletonBase<MonsterManager>
{
public:
    void Init();
    void Update(float deltaTime);
    void Render(stbD2DRenderer& renderer);
    void RenderBossWarnings(stbD2DRenderer& renderer);
    void RenderBossRootBursts(stbD2DRenderer& renderer);
    void StartBossPattern(int instanceId, int patternId, const stb::math::Vector2& center, float radius, int telegraphMs);


    void SpawnMonster(const MonsterSpawnInfo& info);
    void RemoveMonster(int instanceId);
    void ClearMonsters();
    void ApplyServerUpdate(const MonsterUpdateInfo& info);
    void ApplyAttackResult(const AttackResult& result);
    void RespawnMonster(const MonsterUpdateInfo& info);

    Monster* FindMonster(int instanceId);
    //void ApplyMonsterDamage(const MonsterHitInfo& info);
    void Clear(); 

private:
    struct BossPatternVisual
    {
        int instanceId = 0;
        stb::math::Vector2 center{};
        float radius = 0.0f;
        float telegraphSeconds = 0.0f;
        float elapsed = 0.0f;
    };

    std::unordered_map<int, std::unique_ptr<Monster>> m_monsters;
    std::vector<BossPatternVisual> m_bossPatterns;
};

