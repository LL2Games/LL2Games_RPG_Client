//#include "pch.h"

#include "MySocket.h"
#include "../../LL2_Client_Win_Source/stbLogger.h"

CMySocket::CMySocket(CDialogEx* pDlg, e_Status eStatus) : 
    m_dlg(pDlg), 
    m_status(eStatus),
    m_bWorldPhase(false),
    m_bLoginPhase(0),
    m_bChatSocket(0)
//CMySocket::CMySocket()
{
    m_bConnect = FALSE;
    m_bRegister = FALSE;
}
CMySocket::~CMySocket() {}
 
void CMySocket::OnReceive(int nErrorCode)
{
        std::string buf = RecevieBuff();
        size_t len = buf.size();

        M_LOGGER("m_bLoginPhase[%d]", m_bLoginPhase);

        switch (m_status)
        {

        //회원가입
        case E_REGISTER:
        {
            CLogin* pLoginDlg = (CLogin*)m_dlg;
            pLoginDlg->m_pRegDlg->OnRegister(buf.c_str(), len);
            break;
        }

        //로그인
        case E_LOGIN:
        {
            CLogin* pLoginDlg = (CLogin*)m_dlg;
            m_bLoginPhase = FALSE;
            pLoginDlg->OnLogin(buf.c_str(), len);
            break;
        }

        //World초기화
        case E_WORLD_INIT:
        {
            CWorld* pWorldDlg = (CWorld*)m_dlg;
            pWorldDlg->OnInitWorld(buf.c_str(), len);
            break;
        }

        //캐릭터 리스트
        case E_WORLD_CHAR_LIST:
        {
            CWorld* pWorldDlg = (CWorld*)m_dlg;
            pWorldDlg->OnCharacterList(buf.c_str(), len);
            break;
        }

        //채널선택
        case E_WORLD_CHANNEL_SELECT:
        {
            CWorld* pWorldDlg = (CWorld*)m_dlg;
            pWorldDlg->OnChannelSelect(buf.c_str(), len);
            break;
        }

        }
       
        M_LOGGER("recvBuff[%s]", buf.c_str());

        CAsyncSocket::OnReceive(nErrorCode);
}

void CMySocket::OnConnect(int nErrorCode)
{
        if (nErrorCode == 0) {
            AfxMessageBox(_T("서버 연결 성공!"));
            m_bConnect = TRUE;
        }
        else {
            CString msg;
            msg.Format(_T("서버 연결 실패: %d"), nErrorCode);
            AfxMessageBox(msg);
        }

        switch (m_status)
        {
        case E_LOGIN:
        {
            CLogin* pLoginDlg = (CLogin*)m_dlg;
            pLoginDlg->OnSocketConnect(m_bConnect);
            break;
        }

        case E_WORLD_INIT:
        {
            CWorld* pWorldDlg = (CWorld*)m_dlg;
            pWorldDlg->OnSocketConnect(m_bConnect);
            break;
        }

        }
        
        CAsyncSocket::OnConnect(nErrorCode);
}

BOOL CMySocket::connect(const CString &strHost, const int nPort)
{
    BOOL bRet;
    
    // 1. Socket 생성
    bRet = this->Create();
    if (bRet != TRUE)
    {
        int nErr = this->GetLastError();

        CString msg;
        msg.Format(_T("[CMySocket::connect] Create() 실패. err=%d"), nErr);
        AfxMessageBox(msg);

        return FALSE;
    }

    // 2. 서버 연결
    bRet = this->Connect(strHost, nPort);
    if (bRet != TRUE)
    {
        int nErr = this->GetLastError();

        if (nErr == WSAEWOULDBLOCK)
        {
            TRACE(_T("[CMySocket::connect] Connect 진행 중. host=%s, port=%d, err=%d\n"),
                strHost.GetString(), nPort, nErr);

            // 비동기 연결 시도는 정상적으로 시작된 상태
            return TRUE;
        }

        CString msg;
        msg.Format(_T("[CMySocket::connect] Connect() 실패. host=%s, port=%d, err=%d"),
            strHost.GetString(), nPort, nErr);
        AfxMessageBox(msg);

        this->Close();
        return FALSE;
    }

    return TRUE;
}

std::string CMySocket::RecevieBuff()
{
    char temp[PacketLimits::kReceiveChunkSize];
    std::string buffer;

    while (true)
    {
        const int receivedLength = Receive(
            temp,
            static_cast<int>(sizeof(temp))
        );

        if (receivedLength > 0)
        {
            buffer.append(temp, receivedLength);
            continue;
        }

        if (receivedLength == 0)
        {
            Disconnect();
            return buffer;
        }

        const int socketError = GetLastError();

        if (socketError == WSAEWOULDBLOCK)
        {
            break;
        }

        M_LOGGER(
            "Receive failed. error=%d",
            socketError
        );

        Disconnect();
        return {};
    }

    return buffer;
}

//void CMySocket::Parse(CchatClientDlg* pDlg)
//{
//    auto& buf = m_recvBuff;
//
//    while (buf.size())
//    {
//        if (buf.size() < sizeof(PacketHeader))
//        {
//            //K_slog_trace(K_SLOG_ERROR, "[%s][%d] buf.size() < sizeof(PacketHeader)", __FUNCTION__, __LINE__);
//      /*      K_slog_trace(K_SLOG_ERROR, "[%s][%d] sizeof(PacketHeader)[%d]", __FUNCTION__, __LINE__, sizeof(PacketHeader));
//            K_slog_trace(K_SLOG_ERROR, "[%s][%d] buf.size[%d]", __FUNCTION__, __LINE__, buf.size());*/
//            return;
//        }
//
//        PacketHeader* hdr = reinterpret_cast<PacketHeader*>(buf.data());
//        uint16_t pktLen = hdr->length;
//
//        if (buf.size() < pktLen)
//        {
//            //K_slog_trace(K_SLOG_ERROR, "[%s] buf.size small pktLen", __FUNCTION__);
//            return;
//        }
//        uint16_t type = hdr->type;
//
//        const char* payload = reinterpret_cast<const char*>(buf.data() + sizeof(PacketHeader));
//        int payloadLen = pktLen - sizeof(PacketHeader);
//        //K_slog_trace(K_SLOG_TRACE, "[%s] type=%x", __FUNCTION__, type);
//
//        auto handler = PacketFactory::Create(type);
//        if (handler)
//            handler->Execute(pDlg, payload, payloadLen);
//
//        buf.erase(buf.begin(), buf.begin() + pktLen);
//    }
//}

BOOL CMySocket::SendSync(const char* buf, int len)
{
    int sent = 0;

    while (sent < len)
    {
        int ret = Send(buf + sent, len - sent);

        if (ret == SOCKET_ERROR)
        {
            int err = GetLastError();
            if (err == WSAEWOULDBLOCK)
            {
                Sleep(1);   // OS 스케줄러에 양보
                continue;
            }
            else
            {
                return FALSE;
            }
        }
        sent += ret;
    }
    return TRUE;
}

bool CMySocket::SendPacket(const std::string& packet)
{
    int totalSent = 0;
    int packetSize = (int)packet.size();

    while (totalSent < packetSize)
    {
        int sent = Send(packet.data() + totalSent, packetSize - totalSent, 0);
        if (sent == SOCKET_ERROR)
        {
            return false;
        }
        totalSent += sent;
    }

    return true;
}

void CMySocket::Disconnect()
{
    if (m_hSocket != INVALID_SOCKET)
    {
        ShutDown(SD_BOTH); // SD_BOTH: 송신/수신 모두 종료
        Close();     // 소켓 핸들 닫기
    }
}
