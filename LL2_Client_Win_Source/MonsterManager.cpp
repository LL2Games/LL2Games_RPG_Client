#include "MonsterManager.h"

#include "stbCamera.h"
#include "stbD2DRenderer.h"
#include "stbRender.h"
#include "stbResourceManager.h"
#include "stbTexture.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float kRootRiseSeconds = 0.28f;
    constexpr float kRootHoldSeconds = 0.17f;
    constexpr float kRootFadeSeconds = 0.25f;
    constexpr float kRootEffectSeconds = kRootRiseSeconds + kRootHoldSeconds + kRootFadeSeconds;
}

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

    for (auto& visual : m_bossPatterns)
        visual.elapsed += deltaTime;

    m_bossPatterns.erase(std::remove_if(m_bossPatterns.begin(), m_bossPatterns.end(),
            [](const BossPatternVisual& visual)
            {
                return visual.elapsed >= visual.telegraphSeconds + kRootEffectSeconds;
            }),
        m_bossPatterns.end());
}

void MonsterManager::StartBossPattern(int instanceId, int patternId, const stb::math::Vector2& center, float radius, int telegraphMs)
{
    if ((patternId != 1 && patternId != 2) ||
        instanceId <= 0 ||
        telegraphMs <= 0 ||
        !std::isfinite(center.x) ||
        !std::isfinite(center.y) ||
        !std::isfinite(radius) ||
        radius <= 0.0f)
    {
        return;
    }

    // 같은 보스의 예고들도 각각 유지한다.
    m_bossPatterns.push_back({
        instanceId,
        center,
        radius,
        static_cast<float>(telegraphMs) / 1000.0f,
        0.0f
        });
}

void MonsterManager::RenderBossWarnings(stbD2DRenderer& renderer)
{
    for (const BossPatternVisual& visual : m_bossPatterns)
    {
        if (visual.elapsed >= visual.telegraphSeconds)
            continue;

        stb::math::Vector2 position = visual.center;
        if (stb::render::mainCamera != nullptr)
            position = stb::render::mainCamera->CalculatePosition(position);

        const float progress = std::clamp(visual.elapsed / visual.telegraphSeconds, 0.0f, 1.0f);

        renderer.FillCircle(position.x, position.y, visual.radius, D2D1::ColorF(0.30f, 0.16f, 0.06f, 0.12f + 0.18f * progress));
        renderer.DrawCircle(position.x, position.y, visual.radius, D2D1::ColorF(1.0f, 0.56f, 0.16f, 0.55f + 0.35f * progress),2.0f + 2.0f * progress);
    }
}

void MonsterManager::RenderBossRootBursts(stbD2DRenderer& renderer)
{
    auto* resourceManager = stb::SingletonBase<stb::ResourceManager>::getInstance();
    auto* texture = resourceManager->Find<stb::Texture>(L"Monster/100200/effect/root_eruption");
    if (texture == nullptr || texture->GetD2DBitmap() == nullptr)
        return;

    const float imageWidth = static_cast<float>(texture->GetWidth());
    const float imageHeight = static_cast<float>(texture->GetHeight());

    for (const BossPatternVisual& visual : m_bossPatterns)
    {
        const float effectElapsed = visual.elapsed - visual.telegraphSeconds;
        if (effectElapsed < 0.0f || effectElapsed >= kRootEffectSeconds)
            continue;

        stb::math::Vector2 position = visual.center;
        if (stb::render::mainCamera != nullptr)
            position = stb::render::mainCamera->CalculatePosition(position);

        // Reveal the bitmap from its ground line upward without stretching it.
        const float reveal = std::clamp(effectElapsed / kRootRiseSeconds, 0.02f, 1.0f);
        const float renderWidth = visual.radius * 2.0f;
        const float renderHeight = visual.radius * 2.0f;
        const float opacity = effectElapsed < kRootRiseSeconds + kRootHoldSeconds ? 1.0f : std::clamp((kRootEffectSeconds - effectElapsed) / kRootFadeSeconds, 0.0f, 1.0f);

        const D2D1_RECT_F source = D2D1::RectF(0.0f, imageHeight * (1.0f - reveal), imageWidth, imageHeight);
        const D2D1_RECT_F destination = D2D1::RectF(
            position.x - renderWidth * 0.5f,
            position.y - renderHeight * reveal,
            position.x + renderWidth * 0.5f,
            position.y);

        renderer.DrawBitmap(texture->GetD2DBitmap(), destination, source, opacity);
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

    if (it != m_monsters.end() && it->second->GetMonsterId() == info.monsterId)
    {
        it->second->ResetFromSpawnInfo(info);
        return;
    }

    auto monster = std::make_unique<Monster>();
    monster->Initialize();
    monster->InitFromSpawn(info);

    std::string DebugMsg = "Monster Spawn \n";

    OutputDebugStringA(DebugMsg.c_str());

    // Instance IDs are local to a map; a different type needs fresh visual data.
    m_monsters[info.instanceId] = std::move(monster);
}

void MonsterManager::ClearMonsters()
{
    m_monsters.clear();
    m_bossPatterns.clear();
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

    if (result.isDead)
    {
        m_bossPatterns.erase(
            std::remove_if(m_bossPatterns.begin(), m_bossPatterns.end(),
                [&result](const BossPatternVisual& visual)
                {
                    return visual.instanceId == result.monster_instance_id;
                }),
            m_bossPatterns.end());
    }
}

void MonsterManager::RespawnMonster(const MonsterUpdateInfo& info)
{
    m_bossPatterns.erase(
        std::remove_if(m_bossPatterns.begin(), m_bossPatterns.end(),
            [&info](const BossPatternVisual& visual)
            {
                return visual.instanceId == info.instanceId;
            }),
        m_bossPatterns.end());

    auto it = m_monsters.find(info.instanceId);

    if (it != m_monsters.end() && it->second->GetMonsterId() == info.monsterId)
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
    m_bossPatterns.clear();

    OutputDebugStringA("[MonsterManager] Clear\n");
}


void MonsterManager::ResetMovementConnections()
{
    for (auto& entry : m_monsters) entry.second->ResetMovementConnection();
}
