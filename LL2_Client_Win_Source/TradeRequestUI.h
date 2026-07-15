#pragma once
#include "UI.h"
#include "stbTexture.h"
#include "Trade_Info.h"
#include <d2d1.h>

class TradeRequestUI : public UI
{
public:
    void Init() override;
    void Update() override;
    void Render(HDC hdc) override;
    void Render(stbD2DRenderer& renderer) override;

    void RenderRequest(stbD2DRenderer& renderer);
    void RenderReqPopUp(stbD2DRenderer& renderer); // 교환 신청창 팝업 렌더링
public:

    void OnChar(wchar_t ch);      // WM_CHAR 연결
    void OnKeyDown(WPARAM key);   // WM_KEYDOWN 연결
    void Backspace();
    void CloseWindow();
    void OnPopUp(const TradeRequestInfo& info);

    bool        IsDone() const { return m_done; }
    std::wstring GetNickname() const { return m_inputBuffer; }


    void HandleLMouseClick(int x, int y);   // 교환신청 수락,거절

private:
    void CloseRequestPopup();         // 교환신청 팝업 종료
private:
    std::string m_targetPlayerId;
    std::wstring m_inputBuffer;
    bool         m_done = false;

    bool         m_requestPopupActive = false; //교환신청 팝업용
    std::string m_requesterId;
    std::wstring m_requesterNameW;
    D2D1_RECT_F m_acceptButtonRect{};   //교환신청 수락버튼
    D2D1_RECT_F m_rejectButtonRect{};   //교환신청 거절버튼

    static constexpr float BOX_W = 300.f;
    static constexpr float BOX_H = 36.f;
    static constexpr int   MAX_LEN = 12;
};
