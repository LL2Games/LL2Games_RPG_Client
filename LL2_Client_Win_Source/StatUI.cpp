#include "StatUI.h"
#include "UIManager.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbD2DRenderer.h"
#include "PlayerManager.h"
#include "stbPlayer.h"
#include "stbApplication.h"
#include "stbInput.h"
#include "StringConvert.h"
#include "StatPacketHandler.h"
#include "playerInfo.h"

#define M_APP stb::SingletonBase<stb::Application>::getInstance()
#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()
#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()


void StatUI::Init()
{
	m_background = M_REMANAGER->Find<stb::Texture>(L"Stat_background");
    m_plusButton = M_REMANAGER->Find<stb::Texture>(L"plus_button");


    if (m_plusButton != nullptr)
    {
        ID2D1Bitmap* bitmap = m_plusButton->GetD2DBitmap();

        if (bitmap != nullptr)
        {
            m_plusButtonSize = bitmap->GetSize();
        }
    }

    UpdatePlayerStat();
	mActive = false;
}

void StatUI::Update()
{
    if (!mActive)
        return;

    // 마우스 좌표 가져오기
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(M_APP->GetHWND(), &pt);

    if (M_INPUT->GetKeyDown(stb::eKeyCode::LButton))
    {
        HandleLMouseClick(pt.x, pt.y);
    }

    if (M_INPUT->GetKey(stb::eKeyCode::LButton))
    {
        HandleDragging(pt.x, pt.y);
    }

    if (M_INPUT->GetKeyUp(stb::eKeyCode::LButton))
    {
        HandleMouseUp();
    }

}

void StatUI::Render(HDC hdc)
{
	(void)hdc;
}

void StatUI::Render(stbD2DRenderer& renderer)
{
	if (!mActive)
		return;
	// 인벤토리 기본 이미지 그리기
	RenderBackGround(renderer);

    RenderPlusButton(renderer);

    RenderStatText(renderer);

    // 테스트용 인벤토리 드래그 영역 그리기
    renderer.DrawRect(
        (float)m_statClickRect.left,
        (float)m_statClickRect.top,
        (float)(m_statClickRect.right - m_statClickRect.left),
        (float)(m_statClickRect.bottom - m_statClickRect.top),
        D2D1::ColorF::Black
    );


}

void StatUI::RenderBackGround(stbD2DRenderer& renderer)
{
    stb::Texture* background = m_background;
    if (background == nullptr)
    {
        OutputDebugStringA("mBackground null\n");
        return;
    }

    ID2D1Bitmap* bitmap = background->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("bitmap null\n");
        return;
    }

    D2D1_SIZE_F size = bitmap->GetSize();

    renderer.DrawBitmap(bitmap,
        (FLOAT)m_statImgPosX,
        (FLOAT)m_statImgPosY,
        size.width,
        size.height,
        1.0f);
}

void StatUI::RenderPlusButton(stbD2DRenderer& renderer)
{
    stb::Texture* background = m_plusButton;
    if (background == nullptr)
    {
        OutputDebugStringA("mBackground null\n");
        return;
    }

    ID2D1Bitmap* bitmap = background->GetD2DBitmap();
    if (bitmap == nullptr)
    {
        OutputDebugStringA("bitmap null\n");
        return;
    }

    const D2D1_SIZE_F buttonSize = bitmap->GetSize();
    const float originX = static_cast<float>(m_statImgPosX);
    const float originY = static_cast<float>(m_statImgPosY);

    float opacity = m_isActivePlusButton ? 1.0f : 0.1f;

    const auto drawButton =
        [&](const D2D1_POINT_2F buttonPos)
        {
            const float x = originX + buttonPos.x;
            const float y = originY + buttonPos.y;

            renderer.DrawBitmap(bitmap,
                x,
                y,
                buttonSize.width,
                buttonSize.height,
                opacity);

#ifdef _DEBUG
            renderer.DrawRect(
                x,
                y,
                buttonSize.width,
                buttonSize.height,
                D2D1::ColorF::Black
            );
#endif
        };

    for (const PlusButtonInfo& button : kPlusButtons)
    {
        drawButton(button.position);
    }
}

void StatUI::RenderStatText(stbD2DRenderer& renderer)
{
    const float baseX = static_cast<float>(m_statImgPosX);
    const float baseY = static_cast<float>(m_statImgPosY);

    const auto drawText =
        [&](const std::wstring& text, const D2D1_RECT_F& localRect)
        {
            const D2D1_RECT_F screenRect = D2D1::RectF(
                baseX + localRect.left,
                baseY + localRect.top,
                baseX + localRect.right,
                baseY + localRect.bottom
            );

            renderer.DrawTextString(
                text,
                screenRect,
                D2D1::ColorF::Black,
                TextStyle::Stat
            );

#ifdef _DEBUG
            renderer.DrawRect(
                screenRect.left,
                screenRect.top,
                screenRect.right - screenRect.left,
                screenRect.bottom - screenRect.top,
                D2D1::ColorF::Black
            );
#endif
        };

    const auto makeCurrentMaxText =
        [](int current, int maximum)
        {
            return std::to_wstring(current)
                + L" / "
                + std::to_wstring(maximum);
        };

    drawText(Convert::Utf8ToWstr(m_statData.name), m_nameTextRect);

    drawText(PlayerTypeUtil::IntToJobWstring(m_statData.job), m_jobTextRect);

    drawText(std::to_wstring(m_statData.level), m_levelTextRect);

    drawText(makeCurrentMaxText(m_statData.hp, m_statData.maxHp), m_hpTextRect);

    drawText(makeCurrentMaxText(m_statData.mp, m_statData.maxMp), m_mpTextRect);

    drawText(std::to_wstring(m_statData.str), m_strTextRect);

    drawText(std::to_wstring(m_statData.dex), m_dexTextRect);

    drawText(std::to_wstring(m_statData.intel), m_intTextRect);

    drawText(std::to_wstring(m_statData.luk), m_luckTextRect);

    drawText(std::to_wstring(m_statData.remainAp), m_remainAPTextRect);
}

bool StatUI::HandlePlusButtonClick(int mouseX, int mouseY)
{
    if (!m_isActivePlusButton || m_statData.remainAp <= 0)
        return false;

    int localX = mouseX - m_statImgPosX;
    int localY = mouseY - m_statImgPosY;

    for (const PlusButtonInfo& button : kPlusButtons)
    {
        D2D1_RECT_F buttonRect = GetPlusButtonRect(button.position, m_plusButtonSize);
        if (IsPointInRect(buttonRect, localX, localY))
        {
            const std::string strType = PlayerTypeUtil::IntToStringStatType(static_cast<int>(button.statType));
            OutputDebugStringA((strType + " Stat up\n").c_str());
            // StatPacketSender를 통해서 Stat + 정보 전달
            StatPacketHandler::SendStatUp(strType);
            return true;
        }
    }
    
    return false;
}

void StatUI::HandleLMouseClick(int mouseX, int mouseY)
{
    if (HandleStatUIClick(mouseX, mouseY))
        return;

    if (HandlePlusButtonClick(mouseX, mouseY))
        return;
}

void StatUI::HandleMouseUp()
{
    m_isStatDragging = false;
}
void StatUI::HandleDragging(int mouseX, int mouseY)
{
    if (!m_isStatDragging)
        return;

    m_statImgPosX = mouseX - m_dragOffsetX;
    m_statImgPosY = mouseY - m_dragOffsetY;

    m_statClickRect.left = static_cast<float>(m_statImgPosX + 15);
    m_statClickRect.top = static_cast<float>(m_statImgPosY + 18);
    m_statClickRect.right = m_statClickRect.left + m_statClickWidth;
    m_statClickRect.bottom = m_statClickRect.top + m_statClickHeight;
}
bool StatUI::HandleStatUIClick(int mouseX, int mouseY)
{
    int localX = mouseX - m_statImgPosX;
    int localY = mouseY - m_statImgPosY;


    if (IsPointInRect(m_statClickRect, mouseX, mouseY))
    {
        m_isStatDragging = true;
        m_dragOffsetX = localX;
        m_dragOffsetY = localY;
        return true;
    }
    return false;
}

bool StatUI::IsPointInRect(const D2D1_RECT_F& tabRect, int mouseX, int mouseY)
{

    return  mouseX >= tabRect.left &&
        mouseX < tabRect.right &&
        mouseY >= tabRect.top &&
        mouseY < tabRect.bottom;
}

void StatUI::UpdatePlayerStat()
{
    stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();

    if (player != nullptr)
    {
        m_statData = player->GetStatViewData();
    }
}

D2D1_RECT_F StatUI::GetPlusButtonRect(D2D1_POINT_2F buttonPos, D2D1_SIZE_F buttonSize)
{
    return D2D1::RectF(
        buttonPos.x,
        buttonPos.y,
        buttonPos.x + buttonSize.width,
        buttonPos.y + buttonSize.height
    );
}

void StatUI::FinishStatUpRequest()
{
    m_isActivePlusButton = (m_statData.remainAp > 0);
}
