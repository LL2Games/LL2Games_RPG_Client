#include "ChatUI.h"

#include "StringConvert.h"
#include "ChatPacketHandler.h"

#include "stbD2DRenderer.h"

void ChatUI::Init()
{
    mActive = true; // 처음엔 로그만 보이고 입력박스는 숨김
    // (로그는 항상 보이도록 mActive = true로 둬도 됨)
}

void ChatUI::ToggleInputFocus()
{
    m_inputActive = !m_inputActive;
    if (!m_inputActive)
    {
        m_inputBuffer.clear();  // 닫을 때 입력 버퍼 초기화
        m_compositionBuffer.clear();
    }
        
}

void ChatUI::AppendChar(wchar_t ch)
{
    if (!m_inputActive) return;
    if (m_inputBuffer.size() >= 100) return;  // 최대 길이 제한
    m_inputBuffer += ch;
}

void ChatUI::Backspace()
{
    if (!m_inputActive || m_inputBuffer.empty()) return;
    m_inputBuffer.pop_back();
}

void ChatUI::SubmitInput()
{
    if (m_inputBuffer.empty()) return;

    // UTF-16 → UTF-8 변환 후 송신
    std::string utf8 = Convert::WstrToUtf8(m_inputBuffer);
    ChatPacketHandler::SendChat(utf8);
    m_inputBuffer.clear();

    ToggleInputFocus();   // 전송 후 입력 모드 종료
}

void ChatUI::AddMessage(const std::wstring& nick, const std::wstring& msg)
{
    std::wstring line = L"[" + nick + L"] " + msg;
    m_logLines.push_back(line);
    if ((int)m_logLines.size() > MAX_LOG_LINES)
        m_logLines.erase(m_logLines.begin());  // 오래된 줄 제거
}

void ChatUI::Render(stbD2DRenderer& renderer)
{
    D2D1_SIZE_F rtSize = renderer.GetRenderTargetSize();
    float screenH = rtSize.height;

    const float boxW = 500.f;
    const float logH = LOG_BOX_HEIGHT;
    const float inputH = INPUT_BOX_HEIGHT;
    const float marginL = 10.f;
    const float marginB = 50.f;

    float logX = marginL;
    float logY = screenH - marginB - logH - inputH;
    float inputX = marginL;
    float inputY = screenH - marginB - inputH;

    // ── 로그 배경 (반투명 검정) ──
    renderer.FillRect(logX, logY, boxW, logH,
        D2D1::ColorF(0.f, 0.f, 0.f, 0.45f));

    // ── 로그 텍스트 ──
    float lineH = logH / MAX_LOG_LINES;
    for (int i = 0; i < (int)m_logLines.size(); ++i)
    {
        float ty = logY + i * lineH;
        D2D1_RECT_F rect = D2D1::RectF(logX + 4, ty, logX + boxW, ty + lineH);
        renderer.DrawTextString(m_logLines[i], rect,
            D2D1::ColorF(D2D1::ColorF::White), TextStyle::Chat);
    }

    // ── 입력박스 (입력 모드일 때만) ──
    if (m_inputActive)
    {
        // 배경
        renderer.FillRect(inputX, inputY, boxW, inputH,
            D2D1::ColorF(0.f, 0.f, 0.f, 0.7f));
        // 테두리
        renderer.DrawRect(inputX, inputY, boxW, inputH,
            D2D1::ColorF(D2D1::ColorF::Yellow), 1.5f);

        // 입력 텍스트 + 커서
        std::wstring display = m_inputBuffer + m_compositionBuffer + L"_";
        D2D1_RECT_F rect = D2D1::RectF(inputX + 4, inputY + 2,
            inputX + boxW, inputY + inputH);
        renderer.DrawTextString(display, rect,
            D2D1::ColorF(D2D1::ColorF::Yellow), TextStyle::Chat);
    }
    else
    {
        // 비활성 상태: 얇은 회색 테두리만
        renderer.DrawRect(inputX, inputY, boxW, inputH,
            D2D1::ColorF(0.5f, 0.5f, 0.5f, 0.4f), 1.f);

        D2D1_RECT_F rect = D2D1::RectF(inputX + 4, inputY + 2,
            inputX + boxW, inputY + inputH);
        renderer.DrawTextString(L"Enter 키를 눌러 채팅", rect,
            D2D1::ColorF(1.0f, 1.0f, 1.0f, 0.6f), TextStyle::Chat);
    }
}

void ChatUI::Update()
{
    if (!mActive)
        return;

    //TODO
}

void ChatUI::SetComposition(const std::wstring& text)
{
    if (!m_inputActive)
        return;

    m_compositionBuffer = text;
}

void ChatUI::CommitComposition(const std::wstring& text)
{
    if (!m_inputActive)
        return;

    for (const wchar_t ch : text)
    {
        if (m_inputBuffer.size() >= 100)
            break;

        m_inputBuffer += ch;
    }

    m_compositionBuffer.clear();
}

void ChatUI::ClearComposition()
{
    m_compositionBuffer.clear();
}
