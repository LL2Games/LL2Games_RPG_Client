// CWorldNewChar.cpp: 구현 파일
//
#include "afxdialogex.h"
#include "CWorldNewChar.h"
#include "resource.h"
#include "MySocket.h"
#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"

// CWorldNewChar 대화 상자

IMPLEMENT_DYNAMIC(CWorldNewChar, CDialogEx)

CWorldNewChar::CWorldNewChar(CMySocket* sock, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WORLD_NEW_CHAR, pParent), m_pSock(sock)
{
	m_bCheckDup = FALSE;
}

CWorldNewChar::~CWorldNewChar()
{
}

void CWorldNewChar::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_NICK, m_editNick);
	DDX_Control(pDX, IDC_COMBO_JOBS, m_comboJobs);
}

BEGIN_MESSAGE_MAP(CWorldNewChar, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CHECK_DUP, &CWorldNewChar::OnBnClickedButtonCheckDup)
	ON_BN_CLICKED(ID_BUTTON_NEW_CHAR, &CWorldNewChar::OnBnClickedButtonNewChar)
END_MESSAGE_MAP()

BOOL CWorldNewChar::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	AfxMessageBox(_T("init"));
	return TRUE;
}

//중복확인 버튼
void CWorldNewChar::OnBnClickedButtonCheckDup()
{
	CString strNick;
	m_editNick.GetWindowText(strNick);
	
	//Nick 예외처리
	if (strNick.GetLength() == 0)
	{
		AfxMessageBox(_T("닉네임을 입력하세요"));
		return;
	}

	CheckDupNick(strNick);
	//EndDialog(IDOK);
}

void CWorldNewChar::CheckDupNick(const CString& strNick)
{
	int rc = EXIT_FAILURE;

	const CStringA nickA(strNick);

	std::vector<std::string> datas;
	std::string body, pkt;

	datas.push_back(std::string(nickA));

	body = PacketParser::MakeBody(datas);
	//pkt = PacketParser::MakePacket(PKT_CHECK_DUP_CHAR, body);

	m_pSock->m_status = E_WORLD_CHECK_DUP_NICK; //OnRegister를 받기 위함
	m_pSock->SendPacket(pkt);

	rc = EXIT_SUCCESS;
}

int CWorldNewChar::OnCheckDupNick(const char* recvBuff, const size_t recvLen)
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
		//더이상 없으면 중단
		//K_slog_trace(K_SLOG_DEBUG, "[%s][%d]gunoo22_TEST", __FUNCTION__, __LINE__);
		return -1;
	}

	if (status == "nok")
	{
		AfxMessageBox(_T("닉네임이 중복되었습니다."));
	}
	else
	{
		AfxMessageBox(_T("닉네임 사용이 가능합니다."));
		m_bCheckDup = TRUE; //중복체크 성공
		m_editNick.SetReadOnly(TRUE); //읽기 전용으로 변경
	}


	return 0;
}

//캐릭터 생성 버튼
void CWorldNewChar::OnBnClickedButtonNewChar()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	AfxMessageBox(_T("생성"));
}
