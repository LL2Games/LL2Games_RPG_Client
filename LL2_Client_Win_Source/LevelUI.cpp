#include "LevelUI.h"
#include "stbD2DRenderer.h"
#include "PlayerManager.h"
#include "Stat.h"
#include "stbResourceManager.h"
#include "StringConvert.h"


#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()


void LevelUI::Init()
{
	m_background = M_RESOURCEMANAGER->Find<stb::Texture>(L"Level_Background");
	m_levelImg = M_RESOURCEMANAGER->Find<stb::Texture>(L"Level_lvLayer");

    for (int i = 0; i < 10; i++)
    {
        std::wstring resourceName = L"Level_";
        resourceName += std::to_wstring(i);

        m_numbers[i] = M_RESOURCEMANAGER->Find<stb::Texture>(resourceName);
    }
}

void LevelUI::Update()
{

}

void LevelUI::Render(HDC /*hdc*/)
{

}

void LevelUI::Render(stbD2DRenderer& renderer)
{
    RenderBackground(renderer);
    RenderLevelImg(renderer);
    RenderLevel(renderer);
    RenderNickName(renderer);
}

void LevelUI::RenderBackground(stbD2DRenderer& renderer)
{
    if (m_background == nullptr)
    {
        OutputDebugStringA("level_m_background null\n");
        return;
    }

    ID2D1Bitmap* bitmap = m_background->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("expUI_bitmap null\n");
        return;
    }

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    // 기준 해상도 대비 UI 전체 스케일
    /*float scaleX = rtSize.width / 1366.0f;
    float scaleY = rtSize.height / 768.0f;
    float scale = min(scaleX, scaleY);*/
        
    float drawWidth = static_cast<float>(m_background->GetWidth() * 1);
    float drawHeight = static_cast<float>(m_background->GetHeight() * 1);

    m_UIRect = UILayout::CalcRect(
        rtSize.width,
        rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::BottomLeft,
        0.0f,
        14.0f
    );

    renderer.DrawBitmap(
        bitmap,
        (FLOAT)m_UIRect.x,
        (FLOAT)m_UIRect.y,
        (FLOAT)m_UIRect.width,
        (FLOAT)m_UIRect.height,
        1.0f
    );

}

void LevelUI::RenderLevelImg(stbD2DRenderer& renderer)
{
    if (m_levelImg == nullptr)
    {
        OutputDebugStringA("m_levelImg null\n");
        return;
    }

    ID2D1Bitmap* bitmap = m_levelImg->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("expUI_bitmap null\n");
        return;
    }

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    // 기준 해상도 대비 UI 전체 스케일
    /*float scaleX = rtSize.width / 1366.0f;
    float scaleY = rtSize.height / 768.0f;
    float scale = min(scaleX, scaleY);*/

    float drawWidth = static_cast<float>(m_levelImg->GetWidth() * 1);
    float drawHeight = static_cast<float>(m_levelImg->GetHeight() * 1);

    m_UIRect = UILayout::CalcRect(
        rtSize.width,
        rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::BottomLeft,
        5.0f,
        20.0f
    );

    renderer.DrawBitmap(
        bitmap,
        (FLOAT)m_UIRect.x,
        (FLOAT)m_UIRect.y,
        (FLOAT)m_UIRect.width,
        (FLOAT)m_UIRect.height,
        1.0f
    );
}

void LevelUI::RenderLevel(stbD2DRenderer& renderer)
{
    auto player = M_PLAYERMANAGER->GetLocalPlayer();

    if (player == nullptr)
    {
        return;
    }

    auto stat = player->GetStat();

    if (stat == nullptr)
    {
        return;
    }

    std::wstring levelStr = std::to_wstring(stat->GetLevel());

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    // 시작 위치 계산용
    stb::Texture* baseTex = m_numbers[0];

    if (baseTex == nullptr)
    {
        return;
    }

    UIRect baseRect = UILayout::CalcRect(
        rtSize.width,
        rtSize.height,
        static_cast<float>(baseTex->GetWidth()),
        static_cast<float>(baseTex->GetHeight()),
        UIAnchor::BottomLeft,
        25.0f,
        20.0f
    );

    float drawX = 0.0f;

    for (wchar_t c : levelStr)
    {
        if (c < L'0' || c > L'9')
        {
            continue;
        }

        int digit = c - L'0';

        stb::Texture* tex = m_numbers[digit];

        if (tex == nullptr)
        {
            continue;
        }

        ID2D1Bitmap* bitmap = tex->GetD2DBitmap();

        if (bitmap == nullptr)
        {
            continue;
        }

        renderer.DrawBitmap(
            bitmap,
            (FLOAT)baseRect.x + drawX,
            (FLOAT)baseRect.y,
            (FLOAT)tex->GetWidth(),
            (FLOAT)tex->GetHeight(),
            1.0f
        );

        drawX += (float)tex->GetWidth();
    }
	



}

void LevelUI::RenderNickName(stbD2DRenderer& renderer)
{
    auto player = M_PLAYERMANAGER->GetLocalPlayer();

    if (player == nullptr)
    {
        OutputDebugStringA("Player is nullptr\n");
        return;
    }

    std::wstring nickName = Convert::StringToWString(player->GetPlayerName());

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    UIRect temp = UILayout::CalcRect(
        rtSize.width,
        rtSize.height,
        100.0f,
        25.0f,
        UIAnchor::BottomLeft,
        60.0f,
        13.0f
    );

    D2D1_RECT_F textRect = UILayout::ToD2DRect(temp);
   
    renderer.DrawTextString(
        nickName,
        textRect,
        D2D1::ColorF(0x9CADB4),
        TextStyle::NickName
    );
    
}
