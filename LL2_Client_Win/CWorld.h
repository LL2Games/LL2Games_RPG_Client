#pragma once
#include "afxdialogex.h"
#include "..\\LL2_Client_Win_Source\\MySocket.h"
#include "CRegister.h"

class CMySocket;

// CWorld 대화 상자

class CWorld : public CDialogEx
{
	DECLARE_DYNAMIC(CWorld)

public:
	CWorld(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	CWorld(CMySocket* sock, CWnd* pParent = nullptr);
	virtual ~CWorld();

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
	
	CEdit m_editCharList;
	CEdit m_editCharId;
	CEdit m_editChannelId;

	virtual BOOL OnInitDialog();

	void OnSocketConnect(BOOL bConnect);

	//Send
	int InitWorld();
	int CharacterList();


	//Receive
	int OnInitWorld(const char* recvBuff, const int recvLen);
	int OnCharacterList(const char* recvBuff, const int recvLen);
	int OnChannelSelect(const char* recvBuff, const int recvLen);

	afx_msg void OnBnClickedButtonEnter();

private:
	std::string m_account_id;
};
