#include "ProjectileManager.h"
#include "stbLogger.h"
#include <cmath>

void ProjectileManager::Update(float deltaTime)
{
    for (auto it = m_projectiles.begin(); it != m_projectiles.end();)
    {
        it->second->Update(deltaTime);
        if (it->second->IsExpired())
        {
            it->second->LogRemoval("range_reached");
            it = m_projectiles.erase(it);
        }
        else ++it;
    }
}

void ProjectileManager::Render(stbD2DRenderer& renderer)
{
    for (auto& projectile : m_projectiles) projectile.second->Render(renderer);
}

void ProjectileManager::SpawnFromServer(const MonsterProjectileData& info)
{
    M_LOGGER("[Projectile receive] instanceId=%d projectileTypeId=%d ownerMonsterId=%d dir=(%.6f,%.6f) pos=(%.6f,%.6f) speed=%.6f range=%.6f",
        info.instanceId, info.projectileId, info.ownerId, info.dirX, info.dirY,
        info.pos.x, info.pos.y, info.speed, info.range);
    if (!std::isfinite(info.dirX) || !std::isfinite(info.dirY) ||
        !std::isfinite(info.pos.x) || !std::isfinite(info.pos.y) ||
        !std::isfinite(info.speed) || !std::isfinite(info.range) ||
        info.instanceId < 0 || info.speed <= 0 || info.range < 0 ||
        (info.dirX == 0 && info.dirY == 0))
    {
        M_LOGGER("[Projectile reject] instanceId=%d reason=invalid_spawn", info.instanceId);
        return;
    }
    if (!m_seenInstanceIds.insert(info.instanceId).second)
    {
        M_LOGGER("[Projectile duplicate] instanceId=%d reason=already_seen_in_map", info.instanceId);
        return;
    }
    auto projectile = std::make_unique<Projectile>();
    projectile->Initialize();
    projectile->InitFromServer(info);
    if (projectile->IsExpired()) projectile->LogRemoval("range_reached");
    else m_projectiles.emplace(info.instanceId, std::move(projectile));
}

Projectile* ProjectileManager::FindProjectile(int instanceId) const
{
    const auto it = m_projectiles.find(instanceId);
    return it == m_projectiles.end() ? nullptr : it->second.get();
}

void ProjectileManager::RemoveProjectile(int instanceId)
{
    const auto it = m_projectiles.find(instanceId);
    if (it != m_projectiles.end())
    {
        it->second->LogRemoval("explicit_remove");
        m_projectiles.erase(it);
    }
}

void ProjectileManager::Clear(const char* reason)
{
    for (const auto& projectile : m_projectiles) projectile.second->LogRemoval(reason);
    m_projectiles.clear();
    m_seenInstanceIds.clear();
    M_LOGGER("[Projectile clear] reason=%s", reason);
}
