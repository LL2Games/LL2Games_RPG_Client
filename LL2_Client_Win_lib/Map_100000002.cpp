#include "Map_100000002.h"
#include "stbObject.h"
#include "stbCamera.h"
#include "stbRender.h"
#include "stbResourceManager.h"
#include "stbPlayer.h"
#include "PlayerManager.h"
#include "stbApplication.h"
#include "Portal.h"

using namespace stb;

#define M_RESOURCEMANAGER SingletonBase<ResourceManager>::getInstance()
#define M_PLAYERMANAGER SingletonBase<PlayerManager>::getInstance()

void stb::Map_100000002::LoadMapResources()
{
	m_background = M_RESOURCEMANAGER->Find<Texture>(L"Forest_ground_3");
	m_BGM = M_RESOURCEMANAGER->Find<AudioClip>(L"BGM_Forest_ground_3");
}

void stb::Map_100000002::CreateColliders()
{
}

void stb::Map_100000002::OnMapEnter()
{
    if (m_BGM)
        m_BGM->Play();

    if (render::mainCamera != nullptr && m_background != nullptr)
    {
        render::mainCamera->SetWorldSize(math::Vector2(
            static_cast<float>(m_background->GetWidth()),
            static_cast<float>(m_background->GetHeight())
        )
        );
        render::mainCamera->SetLookOffset(math::Vector2(0.0f, 150.0f));
    }
}

void stb::Map_100000002::OnMapExit()
{
    if (m_BGM)
        m_BGM->Stop();
}

void stb::Map_100000002::RenderBackground(stbD2DRenderer& renderer)
{
    if (m_background == nullptr || m_background->GetD2DBitmap() == nullptr)
    {
        return;
    }

    math::Vector2 screenPosition = math::Vector2::Zero;

    if (render::mainCamera != nullptr)
    {
        screenPosition = render::mainCamera->CalculatePosition(math::Vector2::Zero);
    }

    renderer.DrawBitmap(
        m_background->GetD2DBitmap(),
        screenPosition.x,
        screenPosition.y,
        static_cast<float>(m_background->GetWidth()),
        static_cast<float>(m_background->GetHeight())
    );
}
