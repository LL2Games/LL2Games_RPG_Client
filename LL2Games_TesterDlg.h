
// LL2Games_TesterDlg.h: 헤더 파일
//

#pragma once
#include "MySocket.h"
#include "CStatusLed.h"


// CLL2GamesTesterDlg 대화 상자
class CLL2GamesTesterDlg : public CDialogEx
{
// 생성입니다.
public:
	CLL2GamesTesterDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LL2GAMES_TESTER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editIP;
	CEdit m_editPort;
	CEdit m_editType;
	CEdit m_editPayload;
	//CEdit m_editResponse;

	MySocket m_socket;

	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonRequest();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonDisconnect();
	afx_msg LRESULT OnReceive(WPARAM wParam, LPARAM lParam);
	//void UpdateConnectionUI(ConnectionState state);
	afx_msg LRESULT UpdateConnectionUI(WPARAM wParam, LPARAM lParam);
	CListBox m_listResponse;
	CStatusLed m_statusLed;
};
