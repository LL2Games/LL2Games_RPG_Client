#pragma once
#include "..\\LL2_Client_Win_Source\\stbScene.h"
#include "..\\LL2_Client_Win_Source\\Monster.h"
#include "Portal.h"

using namespace stb;

class stbD2DRenderer;

class MapScene : public Scene
{
public:
    void Initialize() final override;
    void Update() override;
    void Render(stbD2DRenderer& renderer) override;
    void OnEnter() final override;
    void OnExit() final override;

    Portal* FindPortal(const std::string& portalId) const;
protected:
    virtual int GetMapId() const = 0;
    virtual void LoadMapResources() = 0;
    virtual void CreateColliders() {}

    virtual void OnMapEnter() {}
    virtual void OnMapExit() {}
    virtual void RenderBackground(stbD2DRenderer& renderer) = 0;

private:
    void CreatePortals();
    std::unordered_map<std::string, Portal*> m_portals;
    std::unordered_map<int, std::unique_ptr<Monster>> m_monsters;
};

