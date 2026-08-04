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
#include <algorithm>

//로그인 아이디
extern std::string g_account_id;
//채널 포트
extern std::string g_channel_port;
//로그인 캐릭터 아이디
extern std::string g_char_id;
// 로그인 일회용 토큰
extern std::string g_world_ticket;
// 채널 일회용 토큰
extern std::string g_channel_ticket;


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
	DDX_Control(pDX, IDC_EDIT_CHARLIST, m_editCharList);
	DDX_Control(pDX, IDC_EDIT_WORLD_CHARID, m_editCharId);
	DDX_Control(pDX, IDC_EDIT_WORLD_CHANNELID, m_editChannelId);
}

BOOL CWorld::connect()
{
	//BOOL bRet;
	//CString strHost = _T("100.99.220.45");
	CString strHost = _T("100.108.54.60");
	CString strPort = _T("5500");
	
	return m_pSock->connect(strHost, atoi(CT2A(strPort)));
}


BEGIN_MESSAGE_MAP(CWorld, CDialogEx)
	ON_BN_CLICKED(ID_BUTTON_WORLD_ENTER, &CWorld::OnBnClickedButtonEnter)
END_MESSAGE_MAP()


// CWorld 메시지 처리기

BOOL CWorld::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	/*m_editHost.SetWindowTextW(_T("100.114.42.54"));
	m_editPort.SetWindowTextW(_T("5000"));

	m_editID.SetWindowTextW(_T("admin1"));
	m_editPasswd.SetWindowTextW(_T("1111"));*/

	m_editCharId.SetWindowText(_T("1")); //캐릭터id
	m_editChannelId.SetWindowText(_T("1")); //채널id

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
	m_pSock->m_status = E_WORLD_INIT;

	if (g_world_ticket.size() != 64)
	{
		M_LOGGER("World ticket is missing or invalid");
		AfxMessageBox(_T("World 인증 티켓이 없습니다. 다시 로그인하세요."));
		return EXIT_FAILURE;
	}

	try
	{
		const std::string body = PacketParser::MakeBody({ g_world_ticket });

		const std::string packet = PacketParser::MakePacket(PKT_INIT_WORLD, body);

		if (!m_pSock->SendPacket(packet))
		{
			M_LOGGER("Failed to send World initialization packet");
			AfxMessageBox(_T("World 인증 요청 전송에 실패했습니다."));
			return EXIT_FAILURE;
		}
	}
	catch (const std::length_error& exception)
	{
		M_LOGGER("World initialization packet is too large: %s", exception.what());
		return EXIT_FAILURE;
	}
	catch (const std::exception& exception)
	{
		M_LOGGER("World initialization packet creation failed: %s",exception.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int CWorld::OnInitWorld(const char* recvBuff, const size_t recvLen)
{
	if (recvBuff == nullptr || recvLen == 0)
	{
		AfxMessageBox(_T("World 서버 응답이 비어 있습니다."));
		return EXIT_FAILURE;
	}

	std::vector<char> packetBuffer(recvBuff,recvBuff + recvLen);

	ParseResult parseResult = PacketParser::TryParse(packetBuffer);

	if (parseResult.status != ParseStatus::Complete)
	{
		M_LOGGER("World initialization response parse failed");
		AfxMessageBox(_T("World 서버 응답 패킷이 올바르지 않습니다."));
		return EXIT_FAILURE;
	}

	const ParsedPacket& packet = parseResult.packet;

	if (packet.type != PKT_INIT_WORLD)
	{
		M_LOGGER("Unexpected World initialization packet type: %u", static_cast<unsigned int>(packet.type));
		g_world_ticket.clear();
		AfxMessageBox(_T("잘못된 World 서버 응답입니다."));
		return EXIT_FAILURE;
	}

	std::size_t offset = 0;
	std::string status;
	std::string errorMessage;

	if (!PacketParser::ParseLengthPrefixedString(
		packet.payload.data(),
		packet.payload.size(),
		offset,
		status,
		errorMessage))
	{
		M_LOGGER("World initialization status parse failed: %s", errorMessage.c_str());
		g_world_ticket.clear();
		AfxMessageBox(_T("World 인증 결과를 읽지 못했습니다."));
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

		M_LOGGER("World initialization rejected: %s",serverError.c_str());

		// 실패한 티켓을 다시 사용하지 않음
		g_world_ticket.clear();
		AfxMessageBox(_T("World 인증에 실패했습니다. 다시 로그인하세요."));
		return EXIT_FAILURE;
	}

	if (offset != packet.payload.size())
	{
		M_LOGGER("Unexpected field in World initialization response");
		g_world_ticket.clear();
		AfxMessageBox(_T("World 서버 응답에 알 수 없는 데이터가 있습니다."));
		return EXIT_FAILURE;
	}

	// 서버에서 이미 소비한 일회성 티켓이므로 클라이언트에서도 제거
	g_world_ticket.clear();
	AfxMessageBox(_T("World 인증 성공"));
	CharacterList();
	return EXIT_SUCCESS;
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

int CWorld::OnCharacterList(const char* recvBuff, const size_t recvLen)
{
	//int i;
	//char* context = NULL;
	//char* pLine = NULL;
	int rc = EXIT_FAILURE;
	size_t offset = 0;
	std::string status, errMsg;
	CString strCharList;

	std::string sBuff;
	std::vector<char> vBuff;

	sBuff.append(recvBuff, recvLen);
	vBuff.insert(vBuff.end(), sBuff.begin(), sBuff.end());
	auto pkt = PacketParser::Parse(vBuff);
	if (!pkt.has_value())
	{
		return -1;
	}

	
	//반복하여 캐릭터 닉네임 추출
	while (1)
	{
		std::string char_name;

		if (!PacketParser::ParseLengthPrefixedString(
			pkt->payload.c_str(),
			pkt->payload.size(),
			offset,
			char_name,
			errMsg))
		{
			//더이상 없으면 중단
			//K_slog_trace(K_SLOG_DEBUG, "[%s][%d]gunoo22_TEST", __FUNCTION__, __LINE__);
			break;
		}

		CString wideValue = UTIL::Utf8ToCString(char_name);

		/*CString line;
		line.Format(
			L"FIELD[%d] : %s\r\n",
			fieldIndex++,
			wideValue.GetString()
		);
		m_listResponse.InsertString(nIdxResponse++, line);*/
		
		if (strCharList.GetLength() == 0)
		{
			strCharList.Format(L"%s", wideValue.GetString());
		}
		else
		{
			strCharList.Format(L"%s,%s", strCharList.GetString(), wideValue.GetString());
		}
	}

	m_editCharList.SetWindowText(strCharList);


	rc = EXIT_SUCCESS;
//err:

	if (rc != EXIT_SUCCESS)
	{
		//AfxMessageBox(_T("로그인 실패: 회원가입을 하세요"));
	}
	else
	{
		AfxMessageBox(_T("CharList 성공"));
		//this->CharacterList(); //캐릭터 선택
		//m_pSock->m_bWorldPhase = FALSE; //로그인 끝
		//EndDialog(IDOK);
	}

	return rc;
}

//채널 접속버튼 클릭
void CWorld::OnBnClickedButtonEnter()
{
	CString strCharId;
	CString strChannelId;
	m_editCharId.GetWindowTextW(strCharId);
	m_editChannelId.GetWindowTextW(strChannelId);

	/*std::vector<std::string> payload;
	payload = UTIL::ParsePayload(strChannelId);*/

	const std::string characterId = std::string(CStringA(strCharId));
	const std::string channelId = std::string(CStringA(strChannelId));

	if (characterId.empty() || channelId.empty())
	{
		AfxMessageBox(_T("캐릭터와 채널을 선택해 주세요."));
		return;
	}

	//캐릭터 아이디 전역변수 등록
	g_char_id = CStringA(strCharId);
	
	m_pSock->m_status = E_WORLD_CHANNEL_SELECT;

	try
	{
		const std::string body = PacketParser::MakeBody({characterId,channelId});
		const std::string packet = PacketParser::MakePacket(PKT_SELECT_CHANNEL, body);
		m_pSock->SendPacket(packet);
	}
	catch (const std::length_error& exception)
	{
		M_LOGGER("ChannelSelect 패킷 크기 초과: %s",exception.what());
	}
	catch (const std::exception& exception)
	{
		M_LOGGER("ChannelSelect 패킷 생성 실패: %s",exception.what());
	}

}


int CWorld::OnChannelSelect(const char* recvBuff, const size_t recvLen)
{
	if (recvBuff == nullptr || recvLen == 0)
	{
		AfxMessageBox(_T("잘못된 채널 선택 응답입니다."));
		return EXIT_FAILURE;
	}

	std::vector<char> packetBuffer(recvBuff, recvBuff + recvLen);

	auto packet = PacketParser::Parse(packetBuffer);

	if (!packet.has_value() || packet->type != PKT_SELECT_CHANNEL)
	{
		AfxMessageBox(_T("채널 선택 응답 파싱에 실패했습니다."));
		return EXIT_FAILURE;
	}

	std::size_t offset = 0;
	std::string errorMessage;

	std::string status;
	std::string channelIp;
	std::string channelPort;
	std::string channelState;
	std::string channelTicket;

	const auto parseField = [&](std::string& output)
		{
			return PacketParser::ParseLengthPrefixedString(
				packet->payload.c_str(),
				packet->payload.size(),
				offset,
				output,
				errorMessage
			);
		};

	if (!parseField(status))
	{
		AfxMessageBox(_T("채널 선택 상태 파싱에 실패했습니다."));
		return EXIT_FAILURE;
	}

	if (status != "ok")
	{
		std::string serverError;

		// NOK 응답에 오류 메시지가 있다면 읽는다.
		if (offset < packet->payload.size())
		{
			parseField(serverError);
		}

		if (!serverError.empty())
		{
			AfxMessageBox(UTIL::Utf8ToCString(serverError));
		}
		else
		{
			AfxMessageBox(_T("채널 선택에 실패했습니다."));
		}

		return EXIT_FAILURE;
	}

	if (!parseField(channelIp) || !parseField(channelPort) || !parseField(channelState) || !parseField(channelTicket))
	{
		AfxMessageBox(_T("채널 선택 응답 필드가 부족합니다."));
		return EXIT_FAILURE;
	}

	// 정의되지 않은 추가 데이터도 거부한다.
	if (offset != packet->payload.size())
	{
		AfxMessageBox(_T("채널 선택 응답에 잘못된 데이터가 포함되어 있습니다."));
		return EXIT_FAILURE;
	}

	const bool isValidTicket = channelTicket.size() == 64 && std::all_of(
			channelTicket.begin(),
			channelTicket.end(),
			[](const char value)
			{
				return
					(value >= '0' && value <= '9') ||
					(value >= 'a' && value <= 'f');
			}
		);

	if (!isValidTicket)
	{
		AfxMessageBox(_T("Channel 인증 티켓이 올바르지 않습니다."));
		return EXIT_FAILURE;
	}

	// 다음 Channel 서버 접속에서 사용할 정보를 보관한다.
	g_channel_port = channelPort;
	g_channel_ticket = channelTicket;

	const CString wideIp = UTIL::Utf8ToCString(channelIp);

	const CString widePort = UTIL::Utf8ToCString(channelPort);

	const CString wideState = UTIL::Utf8ToCString(channelState);

	CString message;
	message.Format(_T("Channel Select 성공 IP[%s], PORT[%s], STATE[%s]"),
		wideIp.GetString(),
		widePort.GetString(),
		wideState.GetString()
	);

	AfxMessageBox(message);

	m_pSock->Disconnect();
	EndDialog(IDOK);

	return EXIT_SUCCESS;
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
