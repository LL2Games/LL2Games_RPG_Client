#define NOMINMAX
#include "ExpBarUI.h"
#include "stbD2DRenderer.h"
#include "stbResourceManager.h"
#include "UILayout.h"
#include "PlayerManager.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

void ExpBarUI::Init()
{
    /*m_expBarTextureKeys[1228] =
    {
        L"1228_expBar_backLayer",
        L"1228_expBar_backLayer"
    };

    m_expBarTextureKeys[1280] =
    {
        L"1280_expBar_backLayer",
        L"1380_expBar_backLayer"
    };


    m_expBarTextureKeys[1920] =
    {
        L"1920_expBar_backLayer",
        L"1920_expBar_backLayer"
    };*/

    m_expBarTextureKeys[1366] =
    {
        L"1366_expBar_backLayer",
        L"1366_expBar_gaugeLayer"
    };

    m_background = M_RESOURCEMANAGER->Find<stb::Texture>(m_expBarTextureKeys[1366].layerBack);
    m_gauge =  M_RESOURCEMANAGER->Find<stb::Texture>(m_expBarTextureKeys[1366].gauge);

    for (int i = 0; i < 10; i++)
    {
        std::wstring resourceName = L"Exp_";
        resourceName += std::to_wstring(i);

        m_numbers[i] = M_RESOURCEMANAGER->Find<stb::Texture>(resourceName);
    }
}

void ExpBarUI::Update()
{
}

void ExpBarUI::Render(HDC hdc)
{
   
}

void ExpBarUI::Render(stbD2DRenderer& renderer)
{
    RenderBackground(renderer);
    RenderGuage(renderer);
    RenderExpText(renderer);
}

void ExpBarUI::RenderBackground(stbD2DRenderer& renderer)
{
    if (m_background == nullptr)
    {
        OutputDebugStringA("expUI_m_background null\n");
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
    float scaleX = rtSize.width / 1366.0f;
    float scaleY = rtSize.height / 768.0f;
    float scale = std::min(scaleX, scaleY);

    // int drawWidth = (int)(BASE_BG_WIDTH * scale);
    // int drawHeight = (int)(BASE_BG_HEIGHT * scale);

    int drawWidth = (int)(m_background->GetWidth() * 1);
    int drawHeight = (int)(m_background->GetHeight() * 1);

    m_UIRect = UILayout::CalcRect(
        (int)rtSize.width,
        (int)rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::BottomLeft,
        0,
        1
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

void ExpBarUI::RenderGuage(stbD2DRenderer& renderer)
{
    auto player = M_PLAYERMANAGER->GetLocalPlayer();
    if (player == nullptr)
        return;

    ID2D1Bitmap* bitmap = m_gauge->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("bitmap null\n");
        return;
    }

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    float scaleX = rtSize.width / 1366.0f;
    float scaleY = rtSize.height / 768.0f;
    float scale = std::min(scaleX, scaleY);

    D2D1_SIZE_F bmpSize = bitmap->GetSize();

    int drawWidth = (int)(bmpSize.width * scale);
    int drawHeight = (int)(bmpSize.height * scale);

    // UI 위치 계산
    UIRect uiRect = UILayout::CalcRect(
        (int)rtSize.width,
        (int)rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::BottomLeft,
        0,
        1
    );

    int curExp = player->GetStat()->GetExp();
    int needExp = player->GetStat()->GetNeedExp();

    float expRatio = 0.0f;
    if (needExp > 0)
    {
        expRatio = (float)curExp / (float)needExp;
    }
    expRatio = std::clamp(expRatio, 0.0f, 1.0f);


    float hpDrawWidth = uiRect.width * expRatio;

    // 출력 영역
    D2D1_RECT_F destRect = D2D1::RectF(
        (FLOAT)uiRect.x,
        (FLOAT)uiRect.y,
        (FLOAT)uiRect.x + hpDrawWidth,
        (FLOAT)uiRect.y + (FLOAT)uiRect.height
    );

    // 원본 이미지에서 잘라올 영역
    D2D1_RECT_F srcRect = D2D1::RectF(
        0.0f,
        0.0f,
        bmpSize.width * expRatio,
        bmpSize.height
    );

    renderer.DrawBitmap(
        bitmap,
        destRect,
        srcRect,
        1.0f
    );
}

void ExpBarUI::RenderExpText(stbD2DRenderer& renderer)
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

    double percent = 0.0;

    if (stat->GetNeedExp() > 0)
    {
        percent =
            (static_cast<double>(stat->GetExp()) /
                static_cast<double>(stat->GetNeedExp())) * 100.0;
    }

    std::wstringstream ss;
    ss << stat->GetExp()
        << L"["
        << std::fixed << std::setprecision(2)
        << percent
        << L"%]";

    std::wstring expText = ss.str();

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    UIRect expRect = UILayout::CalcRect(
        static_cast<int>(rtSize.width),
        static_cast<int>(rtSize.height),
        100,    // 텍스트 영역 너비
        10,     // 텍스트 영역 높이
        UIAnchor::CenterBottom,
        0,
        1
    );

    D2D1_RECT_F textRect = D2D1::RectF(
        static_cast<float>(expRect.x),
        static_cast<float>(expRect.y),
        static_cast<float>(expRect.x + expRect.width),
        static_cast<float>(expRect.y + expRect.height)
    );

    renderer.DrawTextString(
        expText,
        textRect,
        D2D1::ColorF::Black,
       // D2D1::ColorF(0x9CADB4),
        TextStyle::EXP
    );

}
