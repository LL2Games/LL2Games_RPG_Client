#pragma once
#include "UI.h"
#include "UILayout.h"
#include <vector>
#include <string>

class stbD2DRenderer;

class ChatUI : public UI
{
public:
    void Init() override;
    void Update() override;
    void Render(stbD2DRenderer& renderer) override;

    // 외부에서 호출
    void ToggleInputFocus();
    bool IsInputActive() const { return m_inputActive; }

    void AppendChar(wchar_t ch);    // WM_CHAR에서 호출
    void Backspace();               // WM_KEYDOWN(VK_BACK)에서 호출
    void SubmitInput();             // Enter 전송 시

    void AddMessage(const std::wstring& nick, const std::wstring& msg);

private:
    void RenderBackground(stbD2DRenderer& renderer);
    void RenderLogLines(stbD2DRenderer& renderer);
    void RenderInputBox(stbD2DRenderer& renderer);

private:
    static constexpr int MAX_LOG_LINES = 8;     // 채팅 로그 최대 줄 수
    static constexpr float LOG_BOX_HEIGHT = 160.f; // 로그 영역 높이
    static constexpr float INPUT_BOX_HEIGHT = 28.f;

    bool m_inputActive = false;
    std::wstring m_inputBuffer;                   // 현재 타이핑 중인 문자열
    std::vector<std::wstring> m_logLines;         // 최근 MAX_LOG_LINES 개
    UIRect m_logRect{};
    UIRect m_inputRect{};
};