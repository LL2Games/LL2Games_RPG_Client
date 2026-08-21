#include "TradeRequestUI.h"
#include "TradePacketHandler.h"
#include "stbApplication.h"
#include "TradeManager.h"
#include "StringConvert.h"
#include "stbInput.h"
//#include "stbResourceManager.h"

#define M_APP stb::SingletonBase<stb::Application>::getInstance()
#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
//#define M_INVENTORYMANAGER stb::SingletonBase<InventoryManager>::getInstance()
//#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()

#define M_TRADEMANAGER stb::SingletonBase<TradeManager>::getInstance()

void TradeRequestUI::Init()
{
    //m_background = M_REMANAGER->Find<stb::Texture>(L"Trade_normal");
    mActive = false;
}


void TradeRequestUI::Update()
{
    if (!mActive) return;

    //TODO
    if (m_done)
    {
        //완료 TODO;
        m_targetPlayerId = Convert::WstrToUtf8(m_inputBuffer);
        TradePacketHandler::SendTradeRequest(m_targetPlayerId);

        m_done = false; //초기화
        this->CloseWindow();
    }

    if (m_requestPopupActive)
    {
        //마우스 이벤트
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(M_APP->GetHWND(), &pt);

        //UpdateButtonState(pt.x, pt.y);

        if (M_INPUT->GetKeyDown(stb::eKeyCode::LButton))
        {
            HandleLMouseClick(pt.x, pt.y);
        }

        /*if (M_INPUT->GetKey(stb::eKeyCode::LButton))
        {
            HandleDragging(pt.x, pt.y);
        }

        if (M_INPUT->GetKeyUp(stb::eKeyCode::LButton))
        {
            HandleMouseUp();
        }

        if (M_INPUT->GetKeyDown(stb::eKeyCode::RButton))
        {
            HandleRMouseClick(pt.x, pt.y);
        }*/
    }
}

void TradeRequestUI::Render(HDC /*hdc*/)
{
}

void TradeRequestUI::RenderReqPopUp(stbD2DRenderer& renderer)
{
    // =========================
    // 2. 교환 신청 수신 팝업
    // =========================
    const float popupW = 360.f;
    const float popupH = 160.f;

    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();

    float px = rtSize.width / 2.f - popupW / 2.f;
    float py = rtSize.height / 2.f - popupH / 2.f;

    // 팝업 배경
    renderer.FillRect(px, py, popupW, popupH,
        D2D1::ColorF(0.f, 0.f, 0.f, 0.85f));

    // 테두리
    renderer.DrawRect(px, py, popupW, popupH,
        D2D1::ColorF(D2D1::ColorF::Yellow), 2.f);

    // 메시지
    std::wstring message = L"'" + m_requesterNameW + L"'님이 교환신청을 하셨습니다.";

    D2D1_RECT_F msgRect = D2D1::RectF(
        px + 20.f,
        py + 30.f,
        px + popupW - 20.f,
        py + 70.f
    );

    renderer.DrawTextString(message, msgRect,
        D2D1::ColorF(D2D1::ColorF::White), TextStyle::Trade);

    // 버튼 위치 저장
    const float btnW = 100.f;
    const float btnH = 36.f;
    const float btnY = py + 100.f;

    m_acceptButtonRect = D2D1::RectF(
        px + 65.f,
        btnY,
        px + 65.f + btnW,
        btnY + btnH
    );

    m_rejectButtonRect = D2D1::RectF(
        px + popupW - 65.f - btnW,
        btnY,
        px + popupW - 65.f,
        btnY + btnH
    );

    // 수락 버튼
    renderer.FillRect(
        m_acceptButtonRect.left,
        m_acceptButtonRect.top,
        m_acceptButtonRect.right - m_acceptButtonRect.left,
        m_acceptButtonRect.bottom - m_acceptButtonRect.top,
        D2D1::ColorF(0.1f, 0.35f, 0.1f, 0.9f)
    );

    renderer.DrawRect(
        m_acceptButtonRect.left,
        m_acceptButtonRect.top,
        m_acceptButtonRect.right - m_acceptButtonRect.left,
        m_acceptButtonRect.bottom - m_acceptButtonRect.top,
        D2D1::ColorF(D2D1::ColorF::LightGreen),
        1.5f
    );

    D2D1_RECT_F acceptTextRect = D2D1::RectF(
        m_acceptButtonRect.left,
        m_acceptButtonRect.top + 7.f,
        m_acceptButtonRect.right,
        m_acceptButtonRect.bottom
    );

    renderer.DrawTextString(L"수락", acceptTextRect,
        D2D1::ColorF(D2D1::ColorF::White), TextStyle::TradeButton);

    // 거절 버튼
    renderer.FillRect(
        m_rejectButtonRect.left,
        m_rejectButtonRect.top,
        m_rejectButtonRect.right - m_rejectButtonRect.left,
        m_rejectButtonRect.bottom - m_rejectButtonRect.top,
        D2D1::ColorF(0.35f, 0.1f, 0.1f, 0.9f)
    );

    renderer.DrawRect(
        m_rejectButtonRect.left,
        m_rejectButtonRect.top,
        m_rejectButtonRect.right - m_rejectButtonRect.left,
        m_rejectButtonRect.bottom - m_rejectButtonRect.top,
        D2D1::ColorF(D2D1::ColorF::IndianRed),
        1.5f
    );

    D2D1_RECT_F rejectTextRect = D2D1::RectF(
        m_rejectButtonRect.left,
        m_rejectButtonRect.top + 7.f,
        m_rejectButtonRect.right,
        m_rejectButtonRect.bottom
    );
    
    renderer.DrawTextString(L"거절", rejectTextRect,
        D2D1::ColorF(D2D1::ColorF::White), TextStyle::TradeButton);
}
void TradeRequestUI::RenderRequest(stbD2DRenderer& renderer)
{
    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();
    float cx = rtSize.width / 2.f - BOX_W / 2.f;
    float cy = rtSize.height / 2.f;

    // 라벨
    D2D1_RECT_F labelRect = D2D1::RectF(cx, cy -BOX_H, cx + BOX_W, cy );
    renderer.DrawTextString(L"Trade Input Nick(Enter)",
        labelRect, D2D1::ColorF(D2D1::ColorF::White), TextStyle::Trade);

    // 입력 박스 배경
    renderer.FillRect(cx, cy, BOX_W, BOX_H,
        D2D1::ColorF(0.f, 0.f, 0.f, 0.7f));

    // 테두리
    renderer.DrawRect(cx, cy, BOX_W, BOX_H,
        D2D1::ColorF(D2D1::ColorF::Yellow), 1.5f);

    // 입력 텍스트 + 커서
    std::wstring display = m_inputBuffer + L"_";
    D2D1_RECT_F textRect = D2D1::RectF(cx + 6.f, cy + 4.f, cx + BOX_W, cy + BOX_H);
    renderer.DrawTextString(display, textRect,
        D2D1::ColorF(D2D1::ColorF::Yellow), TextStyle::Trade);
}
void TradeRequestUI::Render(stbD2DRenderer& renderer)
{
    //교환신청 팝업 렌더링
    if (mActive)
        RenderRequest(renderer);

    //교환신청 팝업 렌더링
    if (m_requestPopupActive)
        RenderReqPopUp(renderer);

    

}

void TradeRequestUI::CloseWindow()
{
    mActive = false;
    m_done = false;
    m_inputBuffer.clear();  // 닫을 때 입력 버퍼 초기화
}

void TradeRequestUI::OnPopUp(const TradeRequestInfo& info)
{
    { char szTemp[2560] = { 0, }; sprintf_s(szTemp, "[%s][%d] gunoo22_TEST reqId[%s]", __FUNCTION__, __LINE__, info.requesterId.c_str()); OutputDebugStringA(szTemp); }
    std::string ansi_requesterName = Convert::Utf8ToAnsi(info.requesterName);
    { char szTemp[2560] = { 0, }; sprintf_s(szTemp, "[%s][%d] gunoo22_TEST reqName[%s]", __FUNCTION__, __LINE__, ansi_requesterName.c_str()); OutputDebugStringA(szTemp); }
    //TODO
    //교환창 팝업
    m_requesterId = info.requesterId;
    m_requesterNameW = Convert::Utf8ToWstr(info.requesterName);
    m_requestPopupActive = true;
    mActive = true;
}

void TradeRequestUI::OnChar(wchar_t ch)
{
    if (!mActive) return;

    if (ch == L'\b')  // 백스페이스
    {
        if (!m_inputBuffer.empty())
            m_inputBuffer.pop_back();
    }
    else if (ch >= L' ' && (int)m_inputBuffer.size() < MAX_LEN)
    {
        m_inputBuffer += ch;
    }
}

void TradeRequestUI::OnKeyDown(WPARAM key)
{
    if (!mActive) return;

    if (key == VK_ESCAPE)
    {
        // 상대방에게 받은 거래 요청 팝업은
        // 수락/거절 버튼으로만 처리한다.
        if (!m_requestPopupActive)
        {
            CloseWindow();
        }

        return;
    }

    if (key == VK_RETURN && !m_inputBuffer.empty())
    {
        m_done = true;
    }
}

void TradeRequestUI::Backspace()
{
    //if (key == VK_RETURN && !m_inputBuffer.empty())
      //  m_done = true;
}

static bool IsPointInRect(int x, int y, const D2D1_RECT_F& rect)
{
    return x >= rect.left &&
        x <= rect.right &&
        y >= rect.top &&
        y <= rect.bottom;
}

void TradeRequestUI::HandleLMouseClick(int x, int y)
{
    if (!m_requestPopupActive)
        return;

    if (IsPointInRect(x, y, m_acceptButtonRect))
    {
        OutputDebugStringA("[TradeRequestUI] Accept clicked\n");

        // 교환 수락 패킷 전송
        TradePacketHandler::SendTradeAccept(m_requesterId);

        CloseRequestPopup();
        return;
    }

    if (IsPointInRect(x, y, m_rejectButtonRect))
    {
        OutputDebugStringA("[TradeRequestUI] Reject clicked\n");

        // 거절 패킷이 따로 있다면 SendTradeReject 사용
        // 없으면 Cancel 패킷으로 처리
        //TradePacketHandler::SendTradeReject(m_requesterId);
        // 또는
        // TradePacketHandler::SendTradeCancel(m_requesterId);

        CloseRequestPopup();
        return;
    }
}

void TradeRequestUI::CloseRequestPopup()
{
    m_requestPopupActive = false;
    mActive = false;

    m_requesterId.clear();
    m_requesterNameW.clear();

    m_acceptButtonRect = D2D1::RectF(0, 0, 0, 0);
    m_rejectButtonRect = D2D1::RectF(0, 0, 0, 0);
}
