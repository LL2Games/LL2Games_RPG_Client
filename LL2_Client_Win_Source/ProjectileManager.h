#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "stbD2DRenderer.h"
#include "MonsterInfo.h"
#include "Projectile.h"
#include <unordered_set>

class ProjectileManager : public stb::SingletonBase<ProjectileManager>
{
public:
    void Update(float deltaTime);
    void Render(stbD2DRenderer& renderer);

    void SpawnFromServer(const MonsterProjectileData& info);
    void RemoveProjectile(int instanceId);
    void Clear(const char* reason = "map_exit");
    Projectile* FindProjectile(int instanceId) const;

private:
    std::unordered_map<int, std::unique_ptr<Projectile>> m_projectiles;
    // Keep retired IDs until map exit so retransmitted spawn packets cannot revive them.
    std::unordered_set<int> m_seenInstanceIds;
};