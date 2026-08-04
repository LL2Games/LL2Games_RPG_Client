#pragma once
#include "afxdialogex.h"

// CWorldNewChar 대화 상자

class CWorldNewChar : public CDialogEx
{
	DECLARE_DYNAMIC(CWorldNewChar)

public:
	CWorldNewChar(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CWorldNewChar();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WORLD_NEW_CHAR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
