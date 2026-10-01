#include "MapScene.h"
#include "stbObject.h"
#include "stbTime.h"
#include "MonsterManager.h"
#include "MapDataManager.h"
#include "GameSession.h"
#include "StringConvert.h"
#include "stbResourceManager.h"
#include "ProjectileManager.h"
#include "SkillEffectManager.h"
#include "MovementDebug.h"

#define M_TIME SingletonBase<Time>::getInstance()
#define M_MONSTERMANAGER SingletonBase<MonsterManager>::getInstance()
#define M_GAMESESSION SingletonBase<GameSession>::getInstance()
#define M_MAPDATAMANAGER stb::SingletonBase<MapDataManager>::getInstance()
#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_PROJECTILEMANAGER stb::SingletonBase<ProjectileManager>::getInstance()
#define M_SKILLEFFECTMANAGER stb::SingletonBase<SkillEffectManager>::getInstance()

void MapScene::Initialize()
{
	LoadMapResources();
	CreateColliders();
	CreatePortals();

	Scene::Initialize();
}

void MapScene::Update()
{
	Scene::Update();
	M_MONSTERMANAGER->Update(M_TIME->GetDeltaTime());
    M_PROJECTILEMANAGER->Update(M_TIME->GetDeltaTime());
    M_SKILLEFFECTMANAGER->Update();
}

void MapScene::Render(stbD2DRenderer& renderer)
{
	RenderBackground(renderer);
	RenderMovementGeometry(renderer);
	M_MONSTERMANAGER->RenderBossWarnings(renderer);
	Scene::Render(renderer);

	M_MONSTERMANAGER->Render(renderer);
	M_MONSTERMANAGER->RenderBossRootBursts(renderer);
    M_PROJECTILEMANAGER->Render(renderer);
    M_SKILLEFFECTMANAGER->Render(renderer);

   
}

void MapScene::OnEnter()
{
	Scene::OnEnter();

	M_GAMESESSION->EnsurePersistentObjects();
	OnMapEnter();;
}

void MapScene::OnExit()
{
    M_SKILLEFFECTMANAGER->Clear();
    M_MONSTERMANAGER->Clear();
    M_PROJECTILEMANAGER->Clear();
	OnMapExit();
	M_MONSTERMANAGER->ClearMonsters();
	Scene::OnExit();
}

void MapScene::CreatePortals()
{
    const MapData* mapData = M_MAPDATAMANAGER->FindMapData(GetMapId());

    if (mapData == nullptr)
        return;

    for (const PortalData& data : mapData->portals)
    {
        Portal* portal =stb::object::Instantiate<Portal>(stb::enums::eLayerType::Floor,data.position);

		portal->SetPortalId(data.id);
		m_portals[data.id] = portal;

        std::wstring textureKey = Convert::Utf8ToWstr(data.texture);
        portal->SetTexture(M_RESOURCEMANAGER->Find<stb::Texture>(textureKey));

        portal->SetRenderSize(data.renderSize);
        portal->SetTriggerHalfSize(data.halfSize);
        portal->SetInteractionRange(data.interactionRange);
        portal->SetPortalId(data.id);
        std::wstring destinationScene = L"Map_" +std::to_wstring(data.destinationMapId);

        portal->SetDestination(destinationScene,data.spawnPosition);
    }
}

Portal* MapScene::FindPortal(const std::string& portalId) const
{
    auto iter = m_portals.find(portalId);

    if (iter == m_portals.end())
        return nullptr;

    return iter->second;
}

void MapScene::RenderMovementGeometry(stbD2DRenderer& renderer)
{
    const auto* map = M_MAPDATAMANAGER->FindMapData(GetMapId());
    if (!map) return;
    auto screen = [](float x, float y)
    {
        stb::math::Vector2 point{x, y};
        return stb::render::mainCamera ? stb::render::mainCamera->CalculatePosition(point) : point;
    };
    for (const auto& c : map->physics.climbables)
    {
        auto top = screen(c.x, c.top), bottom = screen(c.x, c.bottom);
        const D2D1::ColorF brown(D2D1::ColorF::SaddleBrown);
        if (c.ladder)
        {
            renderer.DrawLine(top.x - 9, top.y, bottom.x - 9, bottom.y, brown, 3);
            renderer.DrawLine(top.x + 9, top.y, bottom.x + 9, bottom.y, brown, 3);
            for (float y = top.y; y <= bottom.y; y += 16)
                renderer.DrawLine(top.x - 9, y, top.x + 9, y, brown, 2);
        }
        else renderer.DrawLine(top.x, top.y, bottom.x, bottom.y, brown, 3);
#ifdef _DEBUG
        renderer.DrawRect(top.x - c.grabRange, top.y, c.grabRange * 2, bottom.y - top.y,
            D2D1::ColorF(D2D1::ColorF::Yellow));
#endif
    }
#ifdef _DEBUG
    for (const auto& platform : map->physics.platforms)
    {
        auto left = screen(platform.left, platform.y), right = screen(platform.right, platform.y);
        renderer.DrawLine(left.x, left.y, right.x, right.y, D2D1::ColorF(D2D1::ColorF::Lime), 2);
    }
#endif
}
