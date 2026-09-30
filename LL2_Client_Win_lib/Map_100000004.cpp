#include "Map_100000004.h"
#include "stbCamera.h"
#include "stbRender.h"
#include "stbResourceManager.h"
#include "stbApplication.h"

using namespace stb;

#define M_RESOURCEMANAGER SingletonBase<ResourceManager>::getInstance()

void Map_100000004::LoadMapResources()
{
    m_background = M_RESOURCEMANAGER->Find<Texture>(L"Boss_room_1");
    m_BGM = M_RESOURCEMANAGER->Find<AudioClip>(L"BGM_Forest_ground_1");
}

void Map_100000004::CreateColliders()
{
}

void Map_100000004::OnMapEnter()
{
    if (m_BGM != nullptr)
        m_BGM->Play();

    if (render::mainCamera != nullptr && m_background != nullptr)
    {
        render::mainCamera->SetWorldSize(math::Vector2(
            static_cast<float>(m_background->GetWidth()),
            static_cast<float>(m_background->GetHeight())));
        render::mainCamera->SetLookOffset(math::Vector2(0.0f, 150.0f));
    }
}

void Map_100000004::OnMapExit()
{
    if (m_BGM != nullptr)
        m_BGM->Stop();
}

void Map_100000004::RenderBackground(stbD2DRenderer& renderer)
{
    if (m_background == nullptr || m_background->GetD2DBitmap() == nullptr)
        return;

    math::Vector2 screenPosition = math::Vector2::Zero;

    if (render::mainCamera != nullptr)
        screenPosition = render::mainCamera->CalculatePosition(math::Vector2::Zero);

    renderer.DrawBitmap(
        m_background->GetD2DBitmap(),
        screenPosition.x,
        screenPosition.y,
        static_cast<float>(m_background->GetWidth()),
        static_cast<float>(m_background->GetHeight()));
}
