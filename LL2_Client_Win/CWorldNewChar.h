#pragma once
#include "afxdialogex.h"

class CMySocket;

// CWorldNewChar 대화 상자

class CWorldNewChar : public CDialogEx
{
	DECLARE_DYNAMIC(CWorldNewChar)
private:
	CWorldNewChar(CWnd* pParent = nullptr);   // 표준 생성자입니다.
public:
	CWorldNewChar(CMySocket* sock, CWnd* pParent = nullptr);
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

public:
	CMySocket* m_pSock;
	CEdit m_editNick;
	CComboBox m_comboJobs;
	afx_msg void OnBnClickedButtonCheckDup();
	afx_msg void OnBnClickedButtonNewChar();

	void CheckDupNick(const CString& strNick);
	int OnCheckDupNick(const char* recvBuff, const size_t recvLen);

private:
	BOOL m_bCheckDup;
};
