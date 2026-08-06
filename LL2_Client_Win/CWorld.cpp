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

#include "CWorldNewChar.h"

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

	static CString ConvertChannelStatusCString(int status)
	{
		switch (static_cast<ChannelInfo::e_ChannelInfo>(status))
		{
		case ChannelInfo::e_ChannelInfo::E_Normal:
			return _T("원활");

		case ChannelInfo::e_ChannelInfo::E_Busy:
			return _T("혼잡");

		case ChannelInfo::e_ChannelInfo::E_Full:
			return _T("포화");

		case ChannelInfo::e_ChannelInfo::E_Die:
			return _T("점검");

		default:
			return _T("알 수 없음");
		}
	}
};

CWorld::CWorld(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WORLD, pParent)
{
	m_bConnect = FALSE;
	m_pSock = new CMySocket(this, E_WORLD_INIT);
	m_pRegDlg = new CRegister(m_pSock);
}

CWorld::CWorld(CString strHost, std::string account_id, CWnd* pParent /*=nullptr*/ ) : m_strHost(strHost), m_account_id(account_id), CDialogEx(IDD_WORLD, pParent)
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
	ON_BN_CLICKED(IDC_BUTTON_NEW_CHAR, &CWorld::OnBnClickedButtonNewChar)
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
	m_comboChannel.ResetContent();


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
	int characterSize = 0;

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


	//캐릭터 리스트 초기화
	m_characters.clear();

	if (!PacketParser::ParseNextIntField(
		pkt->payload.c_str(),
		pkt->payload.size(),
		offset,
		characterSize,
		errMsg))
	{
		return -1;
	}

	//반복하여 캐릭터 닉네임 추출
	for (int i = 0; i < characterSize; i++)
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

	//채널 정보 수신
	m_channels.clear(); //기존 채널 정보 초기화
	//반복하여 채널 정보 수신
	for (int channel_id = 1 ; ; channel_id++)
	{
		int channelState;

		if (!PacketParser::ParseNextIntField(
			pkt->payload.c_str(),
			pkt->payload.size(),
			offset,
			channelState,
			errMsg))
		{
			break;
		}

		ChannelInfo channel;
		channel.channel_id = channel_id;
		channel.state = static_cast<ChannelInfo::e_ChannelInfo>(channelState);
		m_channels.push_back(channel);
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
		
		//캐릭터 리스트 
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

		//채널
		m_comboChannel.ResetContent();
		for (int i = 0; i < static_cast<int>(m_channels.size()); ++i)
		{
			CString strChannel;
			strChannel.Format(_T("%d 채널(%s)"), m_channels[i].channel_id, UTIL::ConvertChannelStatusCString((int)m_channels[i].state).GetString());
			m_comboChannel.InsertString(i, strChannel);
		}
		m_comboChannel.SetCurSel(0); //1채널로 초기화
		
	}

	return rc;
}

//채널 접속버튼 클릭
void CWorld::OnBnClickedButtonEnter()
{
	if (m_pSock == nullptr)
	{
		AfxMessageBox(_T("소켓 정보가 없습니다."));
		return;
	}

	//1. 선택된 캐릭터 행 가져오기
	POSITION pos = m_listCharacter.GetFirstSelectedItemPosition();

	if (pos == nullptr)
	{
		AfxMessageBox(_T("접속할 캐릭터를 선택해주세요."));
		return;
	}
	const int selectedRow = m_listCharacter.GetNextSelectedItem(pos);

	//리스트 행과 m_characters 벡터의 인덱스가 동일하다는 전제
	if (selectedRow < 0 || selectedRow >= static_cast<int>(m_characters.size()))
	{
		AfxMessageBox(_T("선택한 캐릭터 정보를 찾을 수 없습니다."));
		return;
	}

	m_selectedCharacterIndex = selectedRow;
	const CharacterInfo& selectedCharacter = m_characters[m_selectedCharacterIndex];
	const long long charId = selectedCharacter.char_id;

	//2. 선택된 채널 가져오기
	const int channelIndex = m_comboChannel.GetCurSel();
	if (channelIndex == CB_ERR)
	{
		AfxMessageBox(_T("접속할 채널을 선택해주세요."));
		return;
	}
	// 콤보박스 0번 인덱스 = 1채널
	// 콤보박스 1번 인덱스 = 2채널
	const int channelId = channelIndex + 1;

	// 3. 전역 캐릭터 ID 등록
	g_char_id = std::to_string(charId);

	// 4. 채널 선택 요청 상태로 변경
	m_pSock->m_status = E_WORLD_CHANNEL_SELECT;

	//test
	if (0)
		return;

	// 5. 패킷 생성 및 전송
	try
	{
		std::vector<std::string> datas;

		// 서버 프로토콜에 맞춰 채널 ID 전달
    datas.push_back(std::to_string(charId)); //캐릭터 id 추가 (티켓생성을 위함)
		datas.push_back(std::to_string(channelId));
		
		std::string body = PacketParser::MakeBody(datas);
		std::string pkt = PacketParser::MakePacket(PKT_SELECT_CHANNEL, body);
		
		m_pSock->SendPacket(pkt);
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

void CWorld::OnBnClickedButtonNewChar()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWorldNewChar dlg(m_pSock, m_account_id);
	if (dlg.DoModal() != IDOK)
	{
		AfxMessageBox(_T("캐릭터 생성 다이얼로그 모달 오류"));
		return;
	}

	//CWorldNewChar 내부에서는 CWorldNewChar로 되었었으므로 다시 CWorld로 돌리기
	m_pSock->m_dlg = this;

	//캐릭터 생성 이후에 다시 캐릭터 리스트 호출
	this->CharacterList(); //캐릭터 선택
}
