#pragma once
#include "afxdialogex.h"
//#include "..\\LL2_Client_Win_Source\\MySocket.h"

class CMySocket;

// CRegister 대화 상자

class CRegister : public CDialogEx
{
	DECLARE_DYNAMIC(CRegister)

public:
	CRegister(CMySocket* sock, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CRegister();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REGISTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	int Register(const CString& strID, const CString& strPW);
	int OnRegister(const char *pID, const size_t nIDLen);

public:
	CMySocket* m_pSock;
	CEdit m_editID;
	CEdit m_editPasswd;
	CEdit m_editPasswdChecked;
	afx_msg void OnBnClickedButtonRegister();
	//CEdit m_editNick;
};
