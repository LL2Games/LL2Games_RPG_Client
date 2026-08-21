// CLogin.cpp: 구현 파일
//

//#include "pch.h"
#include "afxdialogex.h"
#include "CLogin.h"
#include "resource.h"
#include <vector>
#include <algorithm>
#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"
#include "..\\LL2_Client_Win_Source\\stbLogger.h"

#include "MySocket.h"


//서버 호스트(IP)
extern std::string g_server_ip;

//로그인 아이디
extern std::string g_account_id;
// 로그인 일회용 토큰
extern std::string g_world_ticket;

// CLogin 대화 상자

IMPLEMENT_DYNAMIC(CLogin, CDialogEx)

CLogin::CLogin(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN, pParent)
{
	m_strHost = _T("");
	m_strPort = _T("");
	m_bConnect = FALSE;
	m_pSock = new CMySocket(this, E_LOGIN);
	m_pRegDlg = new CRegister(m_pSock);
}

CLogin::CLogin(CMySocket* sock, CWnd* pParent /*=nullptr*/) : m_pSock(sock), CDialogEx(IDD_LOGIN, pParent), m_bConnect(FALSE)
{
	m_strHost = _T("");
	m_strPort = _T("");
	m_pRegDlg = new CRegister(m_pSock);
}

CLogin::~CLogin()
{
}

void CLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_EDIT_HOST, m_editHost);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
	DDX_Control(pDX, IDC_EDIT_PASSWD, m_editPasswd);
	DDX_Control(pDX, IDC_EDIT_ID, m_editID);
	DDX_Control(pDX, IDC_COMBO_HOST, m_comboHost);
}

BOOL CLogin::connect()
{
	//BOOL bRet;
	CString strHost, strPort;

	//m_editHost.GetWindowText(strHost);
	strHost = m_strHost;
	m_editPort.GetWindowText(strPort);
	
	return m_pSock->connect(strHost, atoi(CT2A(strPort)));
}


BEGIN_MESSAGE_MAP(CLogin, CDialogEx)
	ON_BN_CLICKED(ID_BUTTON_LOGIN, &CLogin::OnBnClickedButtonLogin)
	ON_BN_CLICKED(ID_BUTTON_REGISTER, &CLogin::OnBnClickedButtonRegister)
	ON_CBN_SELCHANGE(IDC_COMBO_HOST, &CLogin::OnCbnSelchangeComboHost)
END_MESSAGE_MAP()


// CLogin 메시지 처리기

BOOL CLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	
	// 실제 접속에 사용할 서버 IP
	m_serverHostList =
	{
		_T("13.124.105.189"),
		_T("100.108.54.60"),
		_T("100.114.42.54")
		
	};

	// 사용자에게 표시되는 서버 이름
	m_comboHost.ResetContent();
	const int server1ComboIndex = m_comboHost.AddString(_T("서버1"));
	m_comboHost.SetItemData(server1ComboIndex, 0);

	const int server2ComboIndex = m_comboHost.AddString(_T("서버2"));
	m_comboHost.SetItemData(server2ComboIndex, 1);

	const int awsComboIndex = m_comboHost.AddString(_T("AWS 서울 서버"));
	m_comboHost.SetItemData(awsComboIndex, 2);


	// 서버1을 기본 선택
	constexpr int awsServerIndex = 0;

	m_comboHost.SetCurSel(awsServerIndex);
	m_strHost = m_serverHostList[awsServerIndex];
	//m_comboHost.SetCurSel(0);

	//m_editHost.SetWindowTextW(_T("100.108.54.60"));
	m_editPort.SetWindowTextW(_T("5000"));

	m_editID.SetWindowTextW(_T("test_account_001"));
	m_editPasswd.SetWindowTextW(_T("1111"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

int CLogin::Login()
{
	int rc = EXIT_FAILURE;
	char* buff = NULL;
	int nBuffLen = 0;
	//int nSendLen = 0;
	CString strID, strPasswd;
	std::vector<std::string> datas;
	std::string body, pkt;

	m_editID.GetWindowText(strID);
	m_editPasswd.GetWindowText(strPasswd);

	nBuffLen = strID.GetLength() + strPasswd.GetLength() + 10;
	buff = (char*)calloc(nBuffLen, sizeof(char));
	if (buff == NULL)
	{
		AfxMessageBox(_T("메모리 할당 실패"));
		goto err;
	}
	//nSendLen = sprintf_s(buff, nBuffLen, "%s$%s$", CStringA(strID), CStringA(strPasswd));
	//PacketUtility::SendPacket(m_pSock, PKT_LOGIN, std::string(buff));

	datas.push_back(std::string((CStringA(strID))));
	datas.push_back(std::string((CStringA(strPasswd))));

	g_account_id = CStringA(strID);
	try
	{
		body = PacketParser::MakeBody(datas);
		pkt = PacketParser::MakePacket(PKT_LOGIN, body);

		m_pSock->m_status = E_LOGIN; //OnLogin을 받기 위함
		m_pSock->SendPacket(pkt);
	}
	catch (const std::length_error& e)
	{
		M_LOGGER("CharacterList 패킷 크기 초과: %s", e.what());
		goto err;
	}
	catch (const std::exception& e)
	{
		M_LOGGER("CharacterList 패킷 생성 실패: %s", e.what());
		goto err;
	}
	
		m_editID.GetWindowText(m_strID);
	rc = EXIT_SUCCESS;
err:
	free(buff);
	buff = NULL;

	return rc;
}

int CLogin::OnLogin(const char * recvBuff, const size_t recvLen)
{
    if (recvBuff == nullptr || recvLen == 0)
    {
        AfxMessageBox(_T("로그인 응답이 비어 있습니다."));
        return EXIT_FAILURE;
    }

    std::vector<char> packetBuffer(recvBuff,recvBuff + recvLen);

    ParseResult parseResult =PacketParser::TryParse(packetBuffer);

    if (parseResult.status != ParseStatus::Complete)
    {
        M_LOGGER("Login response packet parse failed");
        AfxMessageBox(_T("로그인 응답 패킷이 올바르지 않습니다."));
        return EXIT_FAILURE;
    }

    const ParsedPacket& packet = parseResult.packet;

    if (packet.type != PKT_LOGIN)
    {
        M_LOGGER("Unexpected login response packet type: %u",static_cast<unsigned int>(packet.type));
        AfxMessageBox(_T("잘못된 로그인 응답입니다."));
        return EXIT_FAILURE;
    }

    std::size_t offset = 0;
    std::string status;
    std::string worldTicket;
    std::string errorMessage;

    if (!PacketParser::ParseLengthPrefixedString(
        packet.payload.data(),
        packet.payload.size(),
        offset,
        status,
        errorMessage))
    {
        M_LOGGER("Login status parse failed: %s",errorMessage.c_str());
        AfxMessageBox(_T("로그인 상태를 읽지 못했습니다."));
        return EXIT_FAILURE;
    }

    if (status != "ok")
    {
        std::string serverError;

        if (offset < packet.payload.size())
        {
            PacketParser::ParseLengthPrefixedString(
                packet.payload.data(),
                packet.payload.size(),
                offset,
                serverError,
                errorMessage
            );
        }

        // 인증 티켓은 출력하지 않고 오류 메시지만 기록
        M_LOGGER("Login rejected: %s",serverError.c_str());
        AfxMessageBox(_T("아이디 또는 비밀번호가 올바르지 않습니다."));
        return EXIT_FAILURE;
    }

    if (!PacketParser::ParseLengthPrefixedString(
        packet.payload.data(),
        packet.payload.size(),
        offset,
        worldTicket,
        errorMessage))
    {
        M_LOGGER("World ticket parse failed: %s",errorMessage.c_str());
        AfxMessageBox(_T("World 인증 티켓을 받지 못했습니다."));
        return EXIT_FAILURE;
    }

    const bool validWorldTicket =
		worldTicket.size() == 64 &&
        std::all_of(worldTicket.begin(),worldTicket.end(),
            [](const char value)
            {
                return
                    (value >= '0' && value <= '9') ||
                    (value >= 'a' && value <= 'f');
            }
        );

    if (!validWorldTicket)
    {
        M_LOGGER("Invalid World ticket format");
        AfxMessageBox(_T("World 인증 티켓 형식이 올바르지 않습니다."));
        return EXIT_FAILURE;
    }

    if (offset != packet.payload.size())
    {
        M_LOGGER("Unexpected field in login response");
        AfxMessageBox(_T("로그인 응답에 알 수 없는 데이터가 있습니다."));
        return EXIT_FAILURE;
    }

    g_world_ticket = worldTicket;

    AfxMessageBox(_T("로그인 성공"));

    m_pSock->m_bLoginPhase = FALSE;
    m_pSock->Disconnect();
    g_account_id = CStringA(m_strID); //전역변수에 id 담기
		g_server_ip = CStringA(m_strHost); //전역변수에 id 담기  
		EndDialog(IDOK);

    return EXIT_SUCCESS;
}

void CLogin::OnSocketConnect(BOOL bConnect)
{
	if (bConnect)
	{
		m_bConnect = TRUE;
		if (m_pSock->m_bRegister)
			OnBnClickedButtonRegister();
		else
			Login();
	}
}

//로그인 버튼 클릭
void CLogin::OnBnClickedButtonLogin()
{
	//m_editHost.GetWindowTextW(m_strHost);

	const int selectedIndex = m_comboHost.GetCurSel();

	if (selectedIndex == CB_ERR ||
		selectedIndex >= static_cast<int>(m_serverHostList.size()))
	{
		AfxMessageBox(_T("접속할 서버를 선택해주세요."));
		return;
	}
	m_strHost = m_serverHostList[selectedIndex];
	m_editPort.GetWindowTextW(m_strPort);

	if (m_bConnect == FALSE)
		connect();
	else
		OnSocketConnect(m_bConnect);
}

//회원가입 버튼 클릭
void CLogin::OnBnClickedButtonRegister()
{
	m_pSock->m_bRegister = TRUE;
	if (m_bConnect == FALSE)
	{
		if (!connect())
		{
			m_pSock->m_bRegister = FALSE;
			OutputDebugStringA("Login Server Connect Fail");
			return;
		}
	}
	else
	{
		m_pRegDlg->DoModal(); //OnRegister 때문에 이렇게 진행
		m_pSock->m_bRegister = FALSE;
	}
}


//서버 호스트 셀 변경시 이벤트 함수
void CLogin::OnCbnSelchangeComboHost()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	const int selectedIndex = m_comboHost.GetCurSel();
	if (selectedIndex == CB_ERR ||
		selectedIndex >= static_cast<int>(m_serverHostList.size()))
	{
		return;
	}

	// 선택만 바꿔도 현재 서버 연결은 종료
	if (m_bConnect)
	{
		m_pSock->Disconnect();
		m_bConnect = FALSE;
	}

	// 다음 로그인 버튼 클릭 시 사용할 서버
	m_strHost = m_serverHostList[selectedIndex];
}
