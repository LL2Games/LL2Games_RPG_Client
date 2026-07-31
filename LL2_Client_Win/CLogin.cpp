// CLogin.cpp: 구현 파일
//

//#include "pch.h"
#include "afxdialogex.h"
#include "CLogin.h"
#include "resource.h"
#include <vector>
#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"
#include "..\\LL2_Client_Win_Source\\stbLogger.h"

#include "MySocket.h"


//로그인 아이디
extern std::string g_account_id;

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
	DDX_Control(pDX, IDC_EDIT_HOST, m_editHost);
	DDX_Control(pDX, IDC_EDIT_PORT, m_editPort);
	DDX_Control(pDX, IDC_EDIT_PASSWD, m_editPasswd);
	DDX_Control(pDX, IDC_EDIT_ID, m_editID);
}

BOOL CLogin::connect()
{
	//BOOL bRet;
	CString strHost, strPort;

	m_editHost.GetWindowText(strHost);
	m_editPort.GetWindowText(strPort);
	
	return m_pSock->connect(strHost, atoi(CT2A(strPort)));
}


BEGIN_MESSAGE_MAP(CLogin, CDialogEx)
	ON_BN_CLICKED(ID_BUTTON_LOGIN, &CLogin::OnBnClickedButtonLogin)
	ON_BN_CLICKED(ID_BUTTON_REGISTER, &CLogin::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// CLogin 메시지 처리기

BOOL CLogin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_editHost.SetWindowTextW(_T("100.108.54.60"));
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
	size_t offset = 0;
	std::string errMsg;
	CString strCharList;

	std::string sBuff;
	std::vector<char> vBuff;
	CString wideValue;

	sBuff.append(recvBuff, recvLen);
	vBuff.insert(vBuff.end(), sBuff.begin(), sBuff.end());
	auto pkt = PacketParser::Parse(vBuff);
	if (!pkt.has_value())
	{
		return -1;
	}

	std::string status;

	if (!PacketParser::ParseLengthPrefixedString(
		pkt->payload.c_str(),
		pkt->payload.size(),
		offset,
		status,
		errMsg))
	{
		return -1;
	}

	if (status == "nok")
	{
		PacketParser::ParseLengthPrefixedString(
			pkt->payload.c_str(),
			pkt->payload.size(),
			offset,
			errMsg,
			errMsg);

		CString strTmp;
		strTmp.Format(_T("로그인 실패: %s"), CString(errMsg.c_str()));
		AfxMessageBox(strTmp);
	}
	else
	{
		AfxMessageBox(_T("로그인 성공"));
		m_pSock->m_bLoginPhase = FALSE; //로그인 끝
		m_pSock->Disconnect(); //연결 끊기
		EndDialog(IDOK);

	}

	return 0;
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
	m_editHost.GetWindowTextW(m_strHost);
	m_editPort.GetWindowTextW(m_strPort);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	//connect();
	//Login();

	//AfxMessageBox(_T("로그인 성공"));
	////		m_pSock->m_bLoginPhase = FALSE; //로그인 끝
	//EndDialog(IDOK);

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


