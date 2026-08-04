// CWorldNewChar.cpp: 구현 파일
//
#include "afxdialogex.h"
#include "CWorldNewChar.h"
#include "resource.h"

// CWorldNewChar 대화 상자

IMPLEMENT_DYNAMIC(CWorldNewChar, CDialogEx)

CWorldNewChar::CWorldNewChar(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WORLD_NEW_CHAR, pParent)
{
}

CWorldNewChar::~CWorldNewChar()
{
}

void CWorldNewChar::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWorldNewChar, CDialogEx)
END_MESSAGE_MAP()

BOOL CWorldNewChar::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	return TRUE;
}

void CWorldNewChar::OnOK()
{
	CDialogEx::OnOK();
}

void CWorldNewChar::OnCancel()
{
	CDialogEx::OnCancel();
}
