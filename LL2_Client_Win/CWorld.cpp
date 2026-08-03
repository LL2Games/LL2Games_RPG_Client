// CWorld.cpp: 구현 파일
//

//#include "pch.h"
#include "afxdialogex.h"
#include "CWorld.h"
#include "resource.h"
#include <vector>
#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\PacketParser.h"
//#include "..\\LL2_Client_Win_Source\\MySocket.h"
#include "MySocket.h"
//#include "UTIL.h"
#include "..\\LL2_Client_Win_Source\\stbLogger.h"

//로그인 아이디
extern std::string g_account_id;
//채널 포트
extern std::string g_channel_port;
//로그인 캐릭터 아이디
extern std::string g_char_id;


// CWorld 대화 상자

IMPLEMENT_DYNAMIC(CWorld, CDialogEx)

namespace UTIL
{
	static CString Utf8ToCString(const std::string& utf8)
	{
		int len = MultiByteToWideChar(
			CP_UTF8, 0,
			utf8.c_str(), (int)utf8.size(),
			nullptr, 0
		);

		CString result;
		wchar_t* buf = result.GetBuffer(len);
		MultiByteToWideChar(
			CP_UTF8, 0,
			utf8.c_str(), (int)utf8.size(),
			buf, len
		);
		result.ReleaseBuffer(len);

		return result;
	}

	static std::string AnsiToUTF8(const std::string& ansiStr)
	{
		// 1. ANSI(CP_ACP → 보통 CP949) → UTF-16
		int wideLen = MultiByteToWideChar(
			CP_ACP,
			0,
			ansiStr.c_str(),
			-1,
			nullptr,
			0
		);

		std::wstring wideStr;
		wideStr.resize(wideLen);

		MultiByteToWideChar(
			CP_ACP,
			0,
			ansiStr.c_str(),
			-1,
			&wideStr[0],
			wideLen
		);

		// 2. UTF-16 → UTF-8
		int utf8Len = WideCharToMultiByte(
			CP_UTF8,
			0,
			wideStr.c_str(),
			-1,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		std::string utf8Str;
		utf8Str.resize(utf8Len - 1);

		WideCharToMultiByte(
			CP_UTF8,
			0,
			wideStr.c_str(),
			-1,
			&utf8Str[0],
			utf8Len,
			nullptr,
			nullptr
		);

		return utf8Str;
	}

	static std::vector<std::string> ParsePayload(const CString& str)
	{
		std::vector<std::string> values;

		if (str.IsEmpty())
			return values;

		CString temp = str;
		int pos = 0;

		while (true)
		{
			CString token = temp.Tokenize(L"$", pos);
			if (token.IsEmpty() && pos == -1)
				break;

			values.emplace_back(CT2A(token));
		}

		return values;
	}
};

CWorld::CWorld(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WORLD, pParent)
{
	m_bConnect = FALSE;
	m_pSock = new CMySocket(this, E_WORLD_INIT);
	m_pRegDlg = new CRegister(m_pSock);
}

CWorld::CWorld(CString strHost, CWnd* pParent /*=nullptr*/ ) : m_strHost(strHost), CDialogEx(IDD_WORLD, pParent)
{
	m_bConnect = FALSE;
	m_pSock = new CMySocket(this, E_WORLD_INIT);
}

CWorld::CWorld(CMySocket* sock, CWnd* pParent /*=nullptr*/) : m_pSock(sock), CDialogEx(IDD_WORLD, pParent), m_bConnect(FALSE)
{
	m_pRegDlg = new CRegister(m_pSock);
}

CWorld::~CWorld()
{
}

void CWorld::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_EDIT_CHARLIST, m_editCharList);
	//DDX_Control(pDX, IDC_EDIT_WORLD_CHARID, m_editCharId);
	//DDX_Control(pDX, IDC_EDIT_WORLD_CHANNELID, m_editChannelId);
	DDX_Control(pDX, IDC_COMBO_SELECT_CHANNEL, m_comboChannel);
	DDX_Control(pDX, IDC_LIST_CTRL, m_listCharacter);
}

BOOL CWorld::connect()
{
	//CString strHost = _T("100.108.54.60");
	CString strPort = _T("5500");
	
	return m_pSock->connect(m_strHost, atoi(CT2A(strPort)));
}


BEGIN_MESSAGE_MAP(CWorld, CDialogEx)
	ON_BN_CLICKED(ID_BUTTON_WORLD_ENTER, &CWorld::OnBnClickedButtonEnter)
	//IDC_COMBO_SELECT_CHANNEL
END_MESSAGE_MAP()


// CWorld 메시지 처리기

BOOL CWorld::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//캐릭터 리스트 초기화
	m_listCharacter.ModifyStyle(0, LVS_REPORT);
	m_listCharacter.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	m_listCharacter.InsertColumn(0, _T("이름"), LVCFMT_LEFT, 160);
	m_listCharacter.InsertColumn(1, _T("레벨"), LVCFMT_RIGHT, 60);
	m_listCharacter.InsertColumn(2, _T("직업"), LVCFMT_RIGHT, 60);

	

	//채널쪽 초기화
	m_comboChannel.AddString(_T("1채널 - 원활"));
	m_comboChannel.AddString(_T("2채널 - 보통"));
	m_comboChannel.AddString(_T("3채널 - 혼잡"));
	m_comboChannel.SetCurSel(0);

	

	//m_editCharId.SetWindowText(_T("1")); //캐릭터id
	//m_editChannelId.SetWindowText(_T("1")); //채널id

	if (m_bConnect == FALSE)
		connect();
	else
		OnSocketConnect(m_bConnect);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CWorld::OnSocketConnect(BOOL bConnect)
{
	if (bConnect)
	{
		m_bConnect = TRUE;
		this->InitWorld();
	}
}


int CWorld::InitWorld()
{
	int rc = EXIT_FAILURE;

	m_pSock->m_status = E_WORLD_INIT;

	std::vector<std::string> datas;
	std::string body, pkt;

	try
	{
		datas.push_back(g_account_id);
		body = PacketParser::MakeBody(datas);
		pkt = PacketParser::MakePacket(PKT_INIT_WORLD, body);

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

	rc = EXIT_SUCCESS;
err:

	return rc;
}

int CWorld::OnInitWorld(const char* recvBuff, const size_t recvLen)
{
	int rc = EXIT_FAILURE;

	size_t offset = 0;
	std::string status, errMsg;
	std::string sBuff;
	std::vector<char> vBuff;

	sBuff.append(recvBuff, recvLen);
	vBuff.insert(vBuff.end(), sBuff.begin(), sBuff.end());
	auto pkt = PacketParser::Parse(vBuff);
	if (!pkt.has_value())
	{
		return -1;
	}

	PacketParser::ParseLengthPrefixedString(pkt->payload.c_str(), pkt->payload.size(), offset, status, errMsg);

	if (status == "nok")
	{
		CString strTmp;
		PacketParser::ParseLengthPrefixedString(recvBuff, recvLen, offset, status, errMsg);
		strTmp.Format(_T("World 초기화 실패: %s"), CString(status.c_str()));
		AfxMessageBox(strTmp);
		goto err;
	}

	rc = EXIT_SUCCESS;
err:

	if (rc != EXIT_SUCCESS)
	{
		//AfxMessageBox(_T("로그인 실패: 회원가입을 하세요"));
	}
	else
	{
		AfxMessageBox(_T("World 성공"));
		this->CharacterList(); //캐릭터 선택
		//m_pSock->m_bWorldPhase = FALSE; //로그인 끝
		//EndDialog(IDOK);
	}

	return rc;
}

int CWorld::CharacterList()
{
	int rc = EXIT_FAILURE;

	m_pSock->m_status = E_WORLD_CHAR_LIST;

	try
	{
		std::vector<std::string> datas;
		std::string body, pkt;

		body = PacketParser::MakeBody({});
		pkt = PacketParser::MakePacket(PKT_SELECT_CHARACTER, body);

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
	
	rc = EXIT_SUCCESS;
err:

	return rc;
}

static CString ConvertJobCString(int nJob)
{
	CString strJob;
	switch (nJob)
	{
	case 1:
		strJob = _T("전사");
		break;

	case 2:
		strJob = _T("궁수");
		break;

	case 3:
		strJob = _T("마법사");
		break;

	case 4:
		strJob = _T("도적");
		break;
	}

	return strJob;
}

int CWorld::OnCharacterList(const char* recvBuff, const size_t recvLen)
{
	int rc = EXIT_FAILURE;
	size_t offset = 0;
	std::string status, errMsg;

	std::string sBuff;
	std::vector<char> vBuff;

	sBuff.append(recvBuff, recvLen);
	vBuff.insert(vBuff.end(), sBuff.begin(), sBuff.end());
	auto pkt = PacketParser::Parse(vBuff);
	if (!pkt.has_value())
	{
		return -1;
	}

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
		strTmp.Format(_T("World CharacterList Error: %s"), CString(errMsg.c_str()));
	}


	//반복하여 캐릭터 닉네임 추출
	while (1)
	{
		std::string char_data;

		if (!PacketParser::ParseLengthPrefixedString(
			pkt->payload.c_str(),
			pkt->payload.size(),
			offset,
			char_data,
			errMsg))
		{
			break;
		}

		//3$god123$8$1
		std::vector<std::string> tokens;
		std::stringstream stream(char_data);
		std::string token;

		while (std::getline(stream, token, '$'))
		{
			tokens.push_back(token);
		}

		const long long charId = std::stoll(tokens[0]);
		const CString name = UTIL::Utf8ToCString(tokens[1]);
		const int level = std::stoi(tokens[2]);
		const int job = std::stoi(tokens[3]);


		CharacterInfo info;
		info.char_id = charId;
		info.name = name;
		info.level = level;
		info.job = job;

		m_characters.push_back(info);
		
	}


	rc = EXIT_SUCCESS;
//err:

	if (rc != EXIT_SUCCESS)
	{
		//AfxMessageBox(_T("로그인 실패: 회원가입을 하세요"));
	}
	else
	{
		AfxMessageBox(_T("CharList 성공"));
		
		m_listCharacter.DeleteAllItems();

		for (int i = 0; i < static_cast<int>(m_characters.size()); ++i)
		{
			const auto& character = m_characters[i];

			const int row = m_listCharacter.InsertItem(i, character.name);

			CString level;
			level.Format(_T("%d"), character.level);
			m_listCharacter.SetItemText(row, 1, level);

			CString job = ConvertJobCString(character.job);
			m_listCharacter.SetItemText(row, 2, job);
		}


		
	}

	return rc;
}

//채널 접속버튼 클릭
void CWorld::OnBnClickedButtonEnter()
{
	CString strCharId;
	CString strChannelId;
	//m_editCharId.GetWindowTextW(strCharId);
	//m_editChannelId.GetWindowTextW(strChannelId);

	/*std::vector<std::string> payload;
	payload = UTIL::ParsePayload(strChannelId);*/

	//캐릭터 아이디 전역변수 등록
	g_char_id = CStringA(strCharId);
	
	m_pSock->m_status = E_WORLD_CHANNEL_SELECT;

	std::string body, pkt;

	try
	{
		std::vector<std::string> datas;
		datas.push_back(std::string(CStringA(strChannelId)));
		//body = PacketParser::MakeBody(payload);
		body = PacketParser::MakeBody(datas);
		pkt = PacketParser::MakePacket(PKT_SELECT_CHANNEL, body);
		//std::string utf8Packet = UTIL::AnsiToUTF8(pkt);
		//m_pSock->SendPacket(utf8Packet);
		m_pSock->SendPacket(pkt);
	}
	catch (const std::length_error& e)
	{
		M_LOGGER("CharacterList 패킷 크기 초과: %s", e.what());
	}
	catch (const std::exception& e)
	{
		M_LOGGER("CharacterList 패킷 생성 실패: %s", e.what());
	}


}


int CWorld::OnChannelSelect(const char* recvBuff, const size_t recvLen)
{
	//int i;
	//char* context = NULL;
	//char* pLine = NULL;
	int rc = EXIT_FAILURE;
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

	std::string status, channel_ip, channel_port;

	//status
	if (!PacketParser::ParseLengthPrefixedString(
		pkt->payload.c_str(),
		pkt->payload.size(),
		offset,
		status,
		errMsg))
	{
		//더이상 없으면 중단
		//K_slog_trace(K_SLOG_DEBUG, "[%s][%d]gunoo22_TEST", __FUNCTION__, __LINE__);
		rc = -1;
		goto err;
	}

	if (status == "nok")
	{
		rc = -1;
		goto err;
	}

	//ip
	if (!PacketParser::ParseLengthPrefixedString(
		pkt->payload.c_str(),
		pkt->payload.size(),
		offset,
		channel_ip,
		errMsg))
	{
		//더이상 없으면 중단
		//K_slog_trace(K_SLOG_DEBUG, "[%s][%d]gunoo22_TEST", __FUNCTION__, __LINE__);
		rc = -1;
		goto err;
	}

	//port
	if (!PacketParser::ParseLengthPrefixedString(
		pkt->payload.c_str(),
		pkt->payload.size(),
		offset,
		channel_port,
		errMsg))
	{
		//더이상 없으면 중단
		//K_slog_trace(K_SLOG_DEBUG, "[%s][%d]gunoo22_TEST", __FUNCTION__, __LINE__);
		rc = -1;
		goto err;
	}



	rc = EXIT_SUCCESS;
err:

	if (rc != EXIT_SUCCESS)
	{
		AfxMessageBox(_T("실패"));
	}
	else
	{
		CString strTmp;
		CString wideValueIp = UTIL::Utf8ToCString(channel_ip);
		CString wideValuePort = UTIL::Utf8ToCString(channel_port);
		strTmp.Format(_T("Channel Select 성공 IP[%s], PORT[%s]"), wideValueIp.GetString(), wideValuePort.GetString());
		AfxMessageBox(strTmp);
		//this->CharacterList(); //캐릭터 선택
		//m_pSock->m_bWorldPhase = FALSE; //로그인 끝
		//EndDialog(IDOK);

		//채널 포트 전역변수 등록
		g_channel_port = CStringA(wideValuePort);

		m_pSock->Disconnect(); //연결 끊기

		//다이얼로그 종료
		EndDialog(IDOK);
	}

	return rc;
}
//
////로그인 버튼 클릭
//void CWorld::OnBnClickedButtonLogin()
//{
//	m_editHost.GetWindowTextW(m_strHost);
//	m_editPort.GetWindowTextW(m_strPort);
//	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
//
//	//connect();
//	//Login();
//
//	//AfxMessageBox(_T("로그인 성공"));
//	////		m_pSock->m_bLoginPhase = FALSE; //로그인 끝
//	//EndDialog(IDOK);
//
//	if (m_bConnect == FALSE)
//		connect();
//	else
//		OnSocketConnect(m_bConnect);
//}
//
