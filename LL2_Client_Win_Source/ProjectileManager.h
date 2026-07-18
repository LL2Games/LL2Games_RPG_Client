#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "stbD2DRenderer.h"
#include "MonsterInfo.h"
#include "Projectile.h"

class ProjectileManager : public stb::SingletonBase<ProjectileManager>
{
public:
    void Update(float deltaTime);
    void Render(stbD2DRenderer& renderer);

    void ApplyServerUpdate(const MonsterProjectileData& info);
    void RemoveProjectile(int instanceId);
    void Clear();

private:
    std::unordered_map<int, std::unique_ptr<Projectile>> m_projectiles;
};