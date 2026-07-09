
// LL2Games_TesterDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "LL2Games_Tester.h"
#include "LL2Games_TesterDlg.h"
#include "afxdialogex.h"
#include "PacketParser.h"
#include "UTIL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLL2GamesTesterDlg 대화 상자



CLL2GamesTesterDlg::CLL2GamesTesterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LL2GAMES_TESTER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLL2GamesTesterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IP, m_editIP);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
	DDX_Control(pDX, IDC_EDIT_TYPE, m_editType);
	DDX_Control(pDX, IDC_EDIT_PAYLOAD, m_editPayload);
	//DDX_Control(pDX, IDC_EDIT_RESPONSE, m_editResponse);
	DDX_Control(pDX, IDC_LIST_RESPONSE, m_listResponse);
	DDX_Control(pDX, IDC_STATIC_STATUS_LED, m_statusLed);
}

BEGIN_MESSAGE_MAP(CLL2GamesTesterDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CLL2GamesTesterDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_REQUEST, &CLL2GamesTesterDlg::OnBnClickedButtonRequest)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CLL2GamesTesterDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CLL2GamesTesterDlg::OnBnClickedButtonDisconnect)
	ON_MESSAGE(WM_SOCKET_RECEIVE, &CLL2GamesTesterDlg::OnReceive)
	ON_MESSAGE(WM_SOCKET_DISCONNECT, &CLL2GamesTesterDlg::UpdateConnectionUI)
END_MESSAGE_MAP()


// CLL2GamesTesterDlg 메시지 처리기

BOOL CLL2GamesTesterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	//winsock초기화
	AfxSocketInit();
	m_socket.m_hWndOwner = this->m_hWnd;

	//레지스트리에서 읽기
	CString strIP = AfxGetApp()->GetProfileString(_T("Connection"), _T("IP"), _T(""));
	CString strPort = AfxGetApp()->GetProfileString(_T("Connection"), _T("Port"), _T(""));

	/*if (strIP.GetLength() == 0)
		strIP = _T("100.114.42.54");
	if (strPort.GetLength() == 0)
		strPort = _T("1234");*/
	m_editIP.SetWindowTextW(strIP);
	m_editPort.SetWindowTextW(strPort);

	/*m_editIP.SetWindowTextW(_T("100.124.14.8"));
	m_editPort.SetWindowTextW(_T("5500"));*/


	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	UpdateConnectionUI((WPARAM)ConnectionState::DISCONNECTED, 0);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CLL2GamesTesterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CLL2GamesTesterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//Connect
void CLL2GamesTesterDlg::OnBnClickedButtonConnect()
{
	CString strIp, strPort;
	if (m_socket.m_bIsConnect == TRUE)
	{
		AfxMessageBox(L"Socket Already connected");
		return;
	}

	UpdateConnectionUI((WPARAM)ConnectionState::CONNECTING, 0);

	m_editIP.GetWindowTextW(strIp);
	m_editPort.GetWindowTextW(strPort);
	if (!m_socket.Create())
	{
		AfxMessageBox(L"Socket Create Failed");
		UpdateConnectionUI((WPARAM)ConnectionState::DISCONNECTED, 0);
		return;
	}

	if (!m_socket.Connect(strIp, atoi(CT2A(strPort))))
	{
		AfxMessageBox(L"Connect Failed");
		UpdateConnectionUI((WPARAM)ConnectionState::DISCONNECTED, 0);
		return;
	}

	m_socket.m_bIsConnect = TRUE;
	UpdateConnectionUI((WPARAM)ConnectionState::CONNECTED, 0);
	AfxMessageBox(L"Connected to SUCCESS");

	//레지스트리에 저장
	AfxGetApp()->WriteProfileString(_T("Connection"), _T("IP"), strIp);
	AfxGetApp()->WriteProfileString(_T("Connection"), _T("Port"), strPort);
}

//DisConnect
void CLL2GamesTesterDlg::OnBnClickedButtonDisconnect()
{
	m_socket.OnClose(0);
}


//Request
void CLL2GamesTesterDlg::OnBnClickedButtonRequest()
{
	CString strType, strPayload;
	uint16_t type = 0;
	std::vector<std::string> payload;
	if (m_socket.m_bIsConnect == FALSE)
	{
		AfxMessageBox(L"Socket Bind Not Executed");
		return;
	}
	m_editType.GetWindowTextW(strType);
	m_editPayload.GetWindowTextW(strPayload);

	type = UTIL::ParseHexType(strType);
	payload = UTIL::ParsePayload(strPayload);

	std::string body = PacketParser::MakeBody(payload);
	std::string packet = PacketParser::MakePacket(type, body);

	m_socket.Send(packet.c_str(), packet.size());
	/*{
		CString strTmp;
		strTmp.Format(_T("요청성공[%d]", type));
		AfxMessageBox(strTmp);
	}*/
}

LRESULT CLL2GamesTesterDlg::OnReceive(WPARAM wParam, LPARAM lParam)
{
	int nIdxResponse = 0;
	int recvLen = (int)wParam;
	std::vector<char>* data = reinterpret_cast<std::vector<char>*>(lParam);
	
	auto pktOpt = PacketParser::Parse(*data);
	if (!pktOpt.has_value())
	{
		AfxMessageBox(_T("Packet Parse failed"));
		return -1;
	}

	//응답박스 비우기
	m_listResponse.ResetContent();

	const ParsedPacket& pkt = pktOpt.value();

	// TYPE 출력 문자열
	CString strType;
	strType.Format(L"TYPE:0x%04X", pkt.type);
	m_listResponse.InsertString(nIdxResponse++, strType);

	m_listResponse.InsertString(nIdxResponse++, _T("PAYLOAD:"));
	// ---- PAYLOAD Length + Value 파싱 ----
	CString strParsedPayload;
	const char* p = pkt.payload.data();
	size_t len = pkt.payload.size();
	size_t offset = 0;
	int fieldIndex = 0;

	while (offset < len)
	{
		// 최소 1바이트(length)는 있어야 함
		if (offset + 1 > len)
			break;

		uint8_t value_len = static_cast<uint8_t>(p[offset]);
		offset += 1;

		// (선택) 예약 바이트 skip
		if (offset < len && p[offset] == 0x00)
			offset += 1;

		if (offset + value_len > len)
			break;

		std::string value(p + offset, value_len);
		offset += value_len;

		CString wideValue = UTIL::Utf8ToCString(value);

		CString line;
		line.Format(
			L"FIELD[%d] : %s\r\n",
			fieldIndex++,
			wideValue.GetString()
		);
		m_listResponse.InsertString(nIdxResponse++, line);
		strParsedPayload += line;
	}

	if (strParsedPayload.IsEmpty())
		strParsedPayload = L"(empty)";

	// ---- RESPONSE 출력 ----
	/*CString response;
	response.Format(
		L"TYPE:%s\r\nPAYLOAD:\r\n%s",
		strType.GetString(),
		strParsedPayload.GetString()
	);
	
	m_listResponse.InsertString(0, response);*/
	//m_editResponse.SetWindowText(response);
err:

	delete data;
	return 0;
}


//EXIT
void CLL2GamesTesterDlg::OnBnClickedButtonExit()
{
	m_socket.Close();
	PostQuitMessage(0);
	exit(0);
}

//상태변경 함수
LRESULT CLL2GamesTesterDlg::UpdateConnectionUI(WPARAM wParam, LPARAM lParam)
{
	ConnectionState state = (ConnectionState)wParam;
	//m_connState = state;

	switch (state)
	{
	case ConnectionState::CONNECTED:
		m_statusLed.SetColor(RGB(0, 200, 0)); // 초록
		SetDlgItemText(IDC_STATIC_STATUS_TEXT, L"연결 중");
		break;

	case ConnectionState::CONNECTING:
		m_statusLed.SetColor(RGB(255, 165, 0)); // 주황
		SetDlgItemText(IDC_STATIC_STATUS_TEXT, L"연결 시도 중");
		break;

	case ConnectionState::DISCONNECTED:
	default:
		m_statusLed.SetColor(RGB(200, 0, 0)); // 빨강
		SetDlgItemText(IDC_STATIC_STATUS_TEXT, L"연결 끊김");
		break;
	}

	return 0;
}