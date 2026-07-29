#pragma once
#include "MapScene.h"
#include "..\\LL2_Client_Win_Source\\stbTexture.h"
#include "stbAudioClip.h"


namespace stb
{
    class Map_100000000 : public MapScene
    {
    protected:
        void LoadMapResources() override;
        void CreateColliders() override;
        void OnMapEnter() override;
        void OnMapExit() override;
        void RenderBackground(stbD2DRenderer& renderer) override;

    protected:
        int GetMapId() const override { return m_mapId; }
    private:
        Texture* m_background = nullptr;
        AudioClip* m_BGM = nullptr;
        static constexpr int m_mapId = 100000000;
    };
};


