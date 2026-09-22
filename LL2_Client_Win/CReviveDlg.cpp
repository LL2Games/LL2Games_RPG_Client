#include "afxdialogex.h"
#include "CReviveDlg.h"

#include "resource.h"
#include "MySocket.h"

#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"
#include "..\\LL2_Client_Win_Source\\Util.h"
#include "..\\LL2_Client_Win_Source\\PlayerDataPacketHandler.h"


IMPLEMENT_DYNAMIC(CReviveDlg, CDialogEx)

BEGIN_MESSAGE_MAP(CReviveDlg, CDialogEx)
END_MESSAGE_MAP()

CReviveDlg::CReviveDlg(CWnd* pParent)
    : CDialogEx(IDD_REVIVE, pParent)
{
}

BOOL CReviveDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    ResetMessage();
    return TRUE;
}

void CReviveDlg::ResetMessage()
{
    SetDlgItemText(
        IDC_REVIVE_MESSAGE,
        L"캐릭터가 사망했습니다.\r\n"
        L"부활 지점에서 부활하시겠습니까?"
    );

    GetDlgItem(IDOK)->EnableWindow(TRUE);
}

void CReviveDlg::OnOK()
{
    // 현재 단계에서는 클릭 연결만 확인합니다.
    OutputDebugStringW(L"[Revive] 확인 버튼 클릭\n");

    GetDlgItem(IDOK)->EnableWindow(FALSE);

    SetDlgItemText(
        IDC_REVIVE_MESSAGE,
        L"부활 요청 중입니다..."
    );

    PlayerDataPacketHandler::SendPlayerRevive();

    // 닫지 않음
    // 서버 응답 대기
}

void CReviveDlg::OnRevive()
{
    OutputDebugStringW(L"[Revive] 부활 완료 - 다이얼로그 닫기\n");
    ShowWindow(SW_HIDE);
}

void CReviveDlg::OnCancel()
{
    // ESC를 눌러도 창이 닫히지 않도록 합니다.
    // CDialogEx::OnCancel()는 호출하지 않습니다.
}
