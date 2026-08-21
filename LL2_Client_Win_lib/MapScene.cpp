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
	Scene::Render(renderer);

	M_MONSTERMANAGER->Render(renderer);
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
