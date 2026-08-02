#include "stbChatNetworkManager.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <iomanip>
#include "..\\LL2_Client_Win_Source\\stbLogger.h"


#pragma comment(lib, "ws2_32.lib")

//
///*
//    윈도우 프로그램에서 소켓 통신을 하기 위해서 필요한 헤더 파일
//    #include <WinSock2.h>
//    - 윈도우 소켓 API 기본 선언
//    - socket, bind, connect, send, recv, closesocket 같은 것들이 포함되어 있다.
//    - SOCKET, sockaddr_in 같은 타입도 있다
//
//
//
//    #include <WS2tcpip.h>
//    - WinSock2.h에서 조금 더 확장된 네트워크 관련 기능
//    - inet_pton : 사람이 알아보기 쉬운 텍스트 형태의 IPv4와 IPv6 주소를 binary 형태로 변환하는 기능
//                  리틀앤디안 문자열을 바이너리 형태인 IPv6 또는 IPv4 주소 빅앤디안 네트워크 주소로 변환해주는 함수
//
//    - inet_ntop : IPv4, IPv6 주소를 문자열로 변환하는 기능
//                  빅엔디안 네트워크정보를 리틀엔디안 문자열로 변환해주는 함수
//*/

namespace stb
{
    ChatNetworkManager::ChatNetworkManager()
        : m_bConnected(false)
        , m_socket()
    {
        // Winsock 초기화
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0)
        {
            OutputDebugStringA("WSAStartup 실패\n");
        }
    }

    ChatNetworkManager::~ChatNetworkManager()
    {
        Disconnect();
        WSACleanup();
    }

    bool ChatNetworkManager::Connect(const std::string& ip, int port, HWND hWnd)
    {
        if (m_bConnected)
        {
            return true;
        }

        // 소켓 생성
        if (!m_socket.Create())
        {
            return false;
        }

        // 비동기 설정
        if (!m_socket.SetAsync(hWnd, WM_CHAT_SOCKET_RECEIVE))
        {
            m_socket.Close();
            return false;
        }

        // 서버 연결
        if (!m_socket.Connect(ip.c_str(), port))
        {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK)
            {
                m_socket.Close();
                return false;
            }
        }

        m_bConnected = true;
        return true;
    }

    void ChatNetworkManager::Disconnect()
    {
        if (m_bConnected)
        {
            m_socket.Close();
            m_bConnected = false;
        }
    }

    void ChatNetworkManager::SendChatInit(const std::string& char_id)
    {
        std::vector<std::string> datas = { char_id };
        SendPacket(PKT_CHAT_INIT, datas);
    }

    void ChatNetworkManager::SendChat(const std::string& msg)
    {
        std::vector<std::string> datas = { msg };
        SendPacket(PKT_CHAT, datas);
    }


    void ChatNetworkManager::SendPacket(uint16_t type, const std::vector<std::string>& datas)
    {
        if (!m_bConnected)
        {
            return;
        }

        try
        {
            std::string body = PacketParser::MakeBody(datas);
            std::string packet = PacketParser::MakePacket(type, body);

            m_socket.SendPacket(packet);
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

    void ChatNetworkManager::ProcessReceivedData()
    {
        // 데이터 수신
        char tempBuf[PacketLimits::kReceiveChunkSize];
        int recvLen = m_socket.Receive(tempBuf, static_cast<int>(sizeof(tempBuf)));

        if (recvLen > 0)
        {
            // 로그 주석 처리 (성능 향상)
            // std::stringstream ss;
            // ss << "[ProcessReceivedData] 수신: " << recvLen << " bytes\n";
            // OutputDebugStringA(ss.str().c_str());

            auto& recvBuffer = m_socket.GetRecvBuffer();
            recvBuffer.insert(recvBuffer.end(), tempBuf, tempBuf + recvLen);

            // 패킷 파싱
            while (true)
            {
                ParseResult parseResult = PacketParser::TryParse(recvBuffer);

                if (parseResult.status == ParseStatus::NeedMoreData)
                {
                    break;
                }

                if (parseResult.status == ParseStatus::InvalidPacket)
                {
                    M_LOGGER("Invalid chat packet received. Connection will be closed.");

                    recvBuffer.clear();
                    Disconnect();
                    return;
                }

                HandlePacket(parseResult.packet);
            }
        }
        else if (recvLen == 0)
        {
            // 연결 종료
            OutputDebugStringA("[ProcessReceivedData] 연결 종료\n");
            Disconnect();
        }
    }

    void ChatNetworkManager::HandlePacket(const ParsedPacket& packet)
    {
        auto it = m_handlers.find(packet.type);
        if (it != m_handlers.end())
        {
            it->second(packet);
        }
    }

    void ChatNetworkManager::RegisterHandler(uint16_t type, PacketHandler handler)
    {
        m_handlers[type] = handler;
    }

    void ChatNetworkManager::UnregisterHandler(uint16_t type)
    {
        m_handlers.erase(type);
    }
}
