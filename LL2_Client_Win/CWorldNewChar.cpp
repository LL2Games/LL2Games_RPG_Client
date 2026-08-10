// CWorldNewChar.cpp: 구현 파일
//
#include "afxdialogex.h"
#include "CWorldNewChar.h"
#include "resource.h"
#include "MySocket.h"
#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"


static BOOL CheckNickValidate(const CString& strName, CString& strErrMsg)
{
	if (strName.GetLength() == 0)
	{
		strErrMsg = _T("닉네임을 입력하세요");
		return FALSE;
	}

	//글자수 제한
	if (strName.GetLength() < 2 || strName.GetLength() > 10)
	{
		strErrMsg = _T("닉네임은 최소 2자리 이상 10자리 이하여야 합니다.");
		return FALSE;
	}

	CStringW wName(strName);  // 유니코드 변환
	for (int i = 0; i < wName.GetLength(); ++i)
	{
		WCHAR ch = wName[i];
		if (!(
			(ch >= L'A' && ch <= L'Z') ||
			(ch >= L'a' && ch <= L'z') ||
			(ch >= L'0' && ch <= L'9') ||
			(ch >= 0xAC00 && ch <= 0xD7A3)   // 한글 완성형
			))
		{
			strErrMsg = _T("닉네임은 영어, 숫자, 한글로만 이루어져야 합니다.");
			return FALSE;
		}
	}

	return TRUE;
}

// CWorldNewChar 대화 상자

IMPLEMENT_DYNAMIC(CWorldNewChar, CDialogEx)

CWorldNewChar::CWorldNewChar(CMySocket* sock, std::string account_id, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WORLD_NEW_CHAR, pParent), m_pSock(sock), m_account_id(account_id)
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
	
	//직업 리스트 초기화
	m_comboJobs.AddString(_T("전사"));
	m_comboJobs.SetCurSel(0);

	return TRUE;
}

//중복확인 버튼
void CWorldNewChar::OnBnClickedButtonCheckDup()
{
	CString strNick;
	CString strErrMsg;
	m_editNick.GetWindowText(strNick);
	
	//Nick 예외처리
	if (!CheckNickValidate(strNick, strErrMsg))
	{
		AfxMessageBox(strErrMsg);
		return;
	}

	CheckDupNick(strNick);
	//EndDialog(IDOK);
}

void CWorldNewChar::CheckDupNick(const CString& strNick)
{
	const CStringA nickA(strNick);

	std::vector<std::string> datas;
	std::string body, pkt;

	datas.push_back(std::string(nickA));

	body = PacketParser::MakeBody(datas);
	pkt = PacketParser::MakePacket(PKT_CHECK_DUP_CHAR, body);

	m_pSock->m_status = E_WORLD_CHECK_DUP_NICK; //OnCheckDupNick를 받기 위함
	m_pSock->m_dlg = this;
	m_pSock->SendPacket(pkt);
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
	CString strNick;
	m_editNick.GetWindowText(strNick);

	//Nick 예외처리
	if (strNick.GetLength() == 0)
	{
		AfxMessageBox(_T("닉네임을 입력하세요"));
		return;
	}

	//중복확인 예외처리
	if (!m_bCheckDup)
	{
		AfxMessageBox(_T("닉네임 중복확인을 먼저 해주세요"));
		return;
	}


	// 선택한 직업 가져오기
	const int jobIndex = m_comboJobs.GetCurSel() + 1; //1부터 시작
	if (jobIndex == CB_ERR)
	{
		AfxMessageBox(_T("직업을 선택해주세요."));
		return;
	}


	GenNewChar(strNick, jobIndex);
}

void CWorldNewChar::GenNewChar(const CString& strNick, const int job)
{
	const CStringA nickA(strNick);

	std::vector<std::string> datas;
	std::string body, pkt;

	datas.push_back(m_account_id);
	datas.push_back(std::string(nickA));
	datas.push_back(std::to_string(job));

	body = PacketParser::MakeBody(datas);
	pkt = PacketParser::MakePacket(PKT_NEW_CHARACTER, body);

	m_pSock->m_status = E_WORLD_NEW_CHARACTER; //OnGenNewChar를 받기 위함
	m_pSock->m_dlg = this;
	m_pSock->SendPacket(pkt);
}

int CWorldNewChar::OnGenNewChar(const char* recvBuff, const size_t recvLen)
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
		AfxMessageBox(_T("캐릭터 생성에 실패했습니다."));
	}
	else
	{
		AfxMessageBox(_T("캐릭터 생성에 성공했습니다."));
		EndDialog(IDOK);
	}
	return 0;
}
