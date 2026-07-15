#define NOMINMAX
#include "HealthBarUI.h"
#include "stbD2DRenderer.h"
#include "stbTexture.h"
#include "stbResourceManager.h"
#include "PlayerManager.h"
#include <algorithm>
#include <cmath>

#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

void HealthBarUI::Init()
{
    m_background = M_RESOURCEMANAGER->Find<stb::Texture>(L"HealthBar_Background");
    m_hpLayer = M_RESOURCEMANAGER->Find<stb::Texture>(L"HealthBar_hpLayer");
    m_mpLayer = M_RESOURCEMANAGER->Find<stb::Texture>(L"HealthBar_mpLayer");
    m_slash = M_RESOURCEMANAGER->Find<stb::Texture>(L"HealthBar_/");

    for (int i = 0; i < 10; i++)
    {
        std::wstring resourceName = L"HealthBar_";
        resourceName += std::to_wstring(i);

        m_numbers[i] = M_RESOURCEMANAGER->Find<stb::Texture>(resourceName);
    }

}


void HealthBarUI::Update()
{

}

void HealthBarUI::Render(stbD2DRenderer& renderer)
{
    RenderBackGround(renderer);
    RenderHpLayer(renderer);
    RenderMpLayer(renderer);
    RenderHpText(renderer);
    RenderMpText(renderer);
}


void HealthBarUI::RenderBackGround(stbD2DRenderer& renderer)
{
    if (m_background == nullptr)
    {
        OutputDebugStringA("m_background null\n");
        return;
    }

    ID2D1Bitmap* bitmap = m_background->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("bitmap null\n");
        return;
    }

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    // 기준 해상도 대비 UI 전체 스케일
    float scaleX = rtSize.width / 1366.0f;
    float scaleY = rtSize.height / 768.0f;
    float scale = std::min(scaleX, scaleY);

    int drawWidth = (int)(bitmap->GetSize().width * scale);
    int drawHeight = (int)(bitmap->GetSize().height * scale);

    m_UIRect = UILayout::CalcRect(
        (int)rtSize.width,
        (int)rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::CenterBottom,
        10,
        15
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

void HealthBarUI::RenderHpLayer(stbD2DRenderer& renderer)
{
    auto player = M_PLAYERMANAGER->GetLocalPlayer();
    if (player == nullptr)
        return;

    ID2D1Bitmap* bitmap = m_hpLayer->GetD2DBitmap();
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
    m_hpBarRect = UILayout::CalcRect(
        (int)rtSize.width,
        (int)rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::CenterBottom,
        10,
        61
    );

    int curHp = player->GetStat()->GetCurHp();
    int maxHp = player->GetStat()->GetMaxHp();

    float hpRatio = 0.0f;
    if (maxHp > 0)
    {
        hpRatio = (float)curHp / (float)maxHp;
    }
    hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);

 
    float hpDrawWidth = m_hpBarRect.width * hpRatio;

    // 출력 영역
    D2D1_RECT_F destRect = D2D1::RectF(
        (FLOAT)m_hpBarRect.x,
        (FLOAT)m_hpBarRect.y,
        (FLOAT)m_hpBarRect.x + hpDrawWidth,
        (FLOAT)m_hpBarRect.y + (FLOAT)m_hpBarRect.height
    );

    // 원본 이미지에서 잘라올 영역
    D2D1_RECT_F srcRect = D2D1::RectF(
        0.0f,
        0.0f,
        bmpSize.width * hpRatio,
        bmpSize.height
    );

    renderer.DrawBitmap(
        bitmap,
        destRect,
        srcRect,
        1.0f
    );

}
void HealthBarUI::RenderMpLayer(stbD2DRenderer& renderer)
{
    auto player = M_PLAYERMANAGER->GetLocalPlayer();
    if (player == nullptr)
        return;

    ID2D1Bitmap* bitmap = m_mpLayer->GetD2DBitmap();
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
    m_mpBarRect = UILayout::CalcRect(
        (int)rtSize.width,
        (int)rtSize.height,
        drawWidth,
        drawHeight,
        UIAnchor::CenterBottom,
        10,
        20
    );

    int curMp = player->GetStat()->GetCurMp();
    int maxMp = player->GetStat()->GetMaxMp();

    float hpRatio = 0.0f;
    if (maxMp > 0)
    {
        hpRatio = (float)curMp / (float)maxMp;
    }
    hpRatio = std::clamp(hpRatio, 0.0f, 1.0f);


    float hpDrawWidth = m_mpBarRect.width * hpRatio;

    // 출력 영역
    D2D1_RECT_F destRect = D2D1::RectF(
        (FLOAT)m_mpBarRect.x,
        (FLOAT)m_mpBarRect.y,
        (FLOAT)m_mpBarRect.x + hpDrawWidth,
        (FLOAT)m_mpBarRect.y + (FLOAT)m_mpBarRect.height
    );

    // 원본 이미지에서 잘라올 영역
    D2D1_RECT_F srcRect = D2D1::RectF(
        0.0f,
        0.0f,
        bmpSize.width * hpRatio,
        bmpSize.height
    );

    renderer.DrawBitmap(
        bitmap,
        destRect,
        srcRect,
        1.0f
    );
}

void HealthBarUI::RenderHpText(stbD2DRenderer& renderer)
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


    std::wstring curHpStr = std::to_wstring(player->GetStat()->GetCurHp());
    std::wstring maxHpStr = std::to_wstring(player->GetStat()->GetMaxHp());
    std::wstring hpBarStr = curHpStr + L" / " + maxHpStr;

    // 문자열 전체 너비 계산
    float totalWidth = 0.0f;
    float maxHeight = 0.0f;

    for (wchar_t c : hpBarStr)
    {
        stb::Texture* tex = nullptr;

        if (c >= L'0' && c <= L'9')
        {
            int digit = c - L'0';
            tex = m_numbers[digit];
        }
        else if (c == L'/')
        {
            tex = m_slash;
        }
        else if (c == L' ')
        {
            totalWidth += 2.0f;
            continue;
        }

        if (tex == nullptr)
            continue;

        totalWidth += static_cast<float>(tex->GetWidth());
        maxHeight = std::max(maxHeight, static_cast<float>(tex->GetHeight()));
    }

    // MP 바 중앙 기준 시작 위치
    float startX = m_hpBarRect.x + (m_hpBarRect.width - totalWidth) / 2.0f;
    float startY = m_hpBarRect.y + (m_hpBarRect.height - maxHeight) / 2.0f;
    startX = std::round(startX);
    startY = std::round(startY);

    float drawX = 0.0f;

    for (wchar_t c : hpBarStr)
    {
        stb::Texture* tex = nullptr;

        if (c >= L'0' && c <= L'9')
        {
            int digit = c - L'0';
            tex = m_numbers[c - L'0'];
        }
        else if (c == L'/')
        {
            tex = m_slash;
        }
        else if (c == L' ')
        {
            drawX += 2.0f;
            continue;
        }
        else
        {
            continue;
        }

        if (tex == nullptr)
            continue;

        ID2D1Bitmap* numBitmap = tex->GetD2DBitmap();

        if (numBitmap == nullptr)
            continue;

        renderer.DrawBitmap(
            numBitmap,
            startX + drawX,
            startY,
            static_cast<FLOAT>(tex->GetWidth()),
            static_cast<FLOAT>(tex->GetHeight()),
            1.0f
        );

        drawX += static_cast<float>(tex->GetWidth());
    }
}

void HealthBarUI::RenderMpText(stbD2DRenderer& renderer)
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

  
    std::wstring curMpStr = std::to_wstring(player->GetStat()->GetCurMp());
    std::wstring maxMpStr = std::to_wstring(player->GetStat()->GetMaxMp());
    std::wstring mpBarStr = curMpStr + L" / " + maxMpStr;

    // 문자열 전체 너비 계산
    float totalWidth = 0.0f;
    float maxHeight = 0.0f;

    for (wchar_t c : mpBarStr)
    {
        stb::Texture* tex = nullptr;

        if (c >= L'0' && c <= L'9')
        {
            int digit = c - L'0';
            tex = m_numbers[digit];
        }
        else if (c == L'/')
        {
            tex = m_slash;
        }
        else if (c == L' ')
        {
            totalWidth += 2.0f;
            continue;
        }

        if (tex == nullptr)
            continue;

        totalWidth += static_cast<float>(tex->GetWidth());
        maxHeight = std::max(maxHeight, static_cast<float>(tex->GetHeight()));
    }

    // MP 바 중앙 기준 시작 위치
    float startX = m_mpBarRect.x + (m_mpBarRect.width - totalWidth) / 2.0f;
    float startY = m_mpBarRect.y + (m_mpBarRect.height - maxHeight) / 2.0f;
    startX = std::round(startX);
    startY = std::round(startY);

    float drawX = 0.0f;

    for (wchar_t c : mpBarStr)
    {
        stb::Texture* tex = nullptr;

        if (c >= L'0' && c <= L'9')
        {
            int digit = c - L'0';
            tex = m_numbers[c - L'0'];
        }
        else if (c == L'/')
        {
            tex = m_slash;
        }
        else if (c == L' ')
        {
            drawX += 2.0f;
            continue;
        }
        else
        {
            continue;
        }

        if (tex == nullptr)
            continue;

        ID2D1Bitmap* numBitmap = tex->GetD2DBitmap();

        if (numBitmap == nullptr)
            continue;

        renderer.DrawBitmap(
            numBitmap,
            startX + drawX,
            startY,
            static_cast<FLOAT>(tex->GetWidth()),
            static_cast<FLOAT>(tex->GetHeight()),
            1.0f
        );

        drawX += static_cast<float>(tex->GetWidth());
    }
}
