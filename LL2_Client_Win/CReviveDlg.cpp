#include "afxdialogex.h"
#include "CReviveDlg.h"

#include "resource.h"
#include "MySocket.h"

#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"
#include "..\\LL2_Client_Win_Source\\Util.h"


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

    // 다음 단계: 여기서 Revive 요청 함수를 호출합니다.
    // 전송을 접수한 경우에만 버튼 비활성화 및 대기 문구 표시.

    // CDialogEx::OnOK()는 호출하지 않습니다.
    // 서버 응답이 올 때까지 창을 유지합니다.
}

void CReviveDlg::OnCancel()
{
    // ESC를 눌러도 창이 닫히지 않도록 합니다.
    // CDialogEx::OnCancel()는 호출하지 않습니다.
}
