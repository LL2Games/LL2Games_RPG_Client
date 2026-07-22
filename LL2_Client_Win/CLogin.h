#pragma once
#include "afxdialogex.h"
#include "..\\LL2_Client_Win_Source\\MySocket.h"
#include "CRegister.h"

class CMySocket;

// CLogin 대화 상자

class CLogin : public CDialogEx
{
	DECLARE_DYNAMIC(CLogin)

public:
	CLogin(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	CLogin(CMySocket* sock, CWnd* pParent = nullptr);
	virtual ~CLogin();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CMySocket* m_pSock;
	CRegister* m_pRegDlg;
	BOOL connect();

	BOOL m_bConnect;
	CEdit m_editHost;
	CEdit m_editPort;
	CString m_strHost;
	CString m_strPort;

	CEdit m_editPasswd;
	CEdit m_editID;
	CString m_strID;
	afx_msg void OnBnClickedButtonLogin();
	afx_msg void OnBnClickedButtonRegister();
	virtual BOOL OnInitDialog();
	void OnSocketConnect(BOOL bConnect);
	int Login();
	int OnLogin(const char *recvBuf, const size_t recvLen);

};
