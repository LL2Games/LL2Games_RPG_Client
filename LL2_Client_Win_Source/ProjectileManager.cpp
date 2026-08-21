#include "ProjectileManager.h"
#include "stbLogger.h"

void ProjectileManager::Update(float deltaTime)
{
    for (auto& projectile : m_projectiles)
    {
        projectile.second->Update(deltaTime);
    }


    //만료된 투사체를 삭제
    for (auto it = m_projectiles.begin(); it != m_projectiles.end();)
    {
        if (it->second->IsExpired())
        {
            it = m_projectiles.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ProjectileManager::Render(stbD2DRenderer& renderer)
{
    for (auto& projectile : m_projectiles)
    {
        projectile.second->Render(renderer);
    }
}

void ProjectileManager::ApplyServerUpdate(const MonsterProjectileData& info)
{
    auto it = m_projectiles.find(info.instanceId);

    if (it == m_projectiles.end())
    {
        auto projectile = std::make_unique<Projectile>();
        projectile->Initialize();
        projectile->InitFromServer(info);

        m_projectiles.emplace(
            info.instanceId,
            std::move(projectile));

        return;
    }

    M_LOGGER("[%f, %f]", info.pos.x, info.pos.y);
    it->second->ApplyServerUpdate(info);
}

void ProjectileManager::RemoveProjectile(int instanceId)
{
    if (instanceId < 0)
        return;

    auto it = m_projectiles.find(instanceId);
    if (it != m_projectiles.end())
    {
        m_projectiles.erase(it);
    }
}

void ProjectileManager::Clear()
{
    m_projectiles.clear();

    OutputDebugStringA("[MonsterManager] Clear\n");
}
