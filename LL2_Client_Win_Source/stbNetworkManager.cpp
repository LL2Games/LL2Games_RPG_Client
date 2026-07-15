#include "stbNetworkManager.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "ws2_32.lib")

/*
    윈도우 프로그램에서 소켓 통신을 하기 위해서 필요한 헤더 파일
    #include <WinSock2.h>
    - 윈도우 소켓 API 기본 선언
    - socket, bind, connect, send, recv, closesocket 같은 것들이 포함되어 있다.
    - SOCKET, sockaddr_in 같은 타입도 있다

 

    #include <WS2tcpip.h>
    - WinSock2.h에서 조금 더 확장된 네트워크 관련 기능
    - inet_pton : 사람이 알아보기 쉬운 텍스트 형태의 IPv4와 IPv6 주소를 binary 형태로 변환하는 기능
                  리틀앤디안 문자열을 바이너리 형태인 IPv6 또는 IPv4 주소 빅앤디안 네트워크 주소로 변환해주는 함수

    - inet_ntop : IPv4, IPv6 주소를 문자열로 변환하는 기능
                  빅엔디안 네트워크정보를 리틀엔디안 문자열로 변환해주는 함수 
*/

namespace stb
{
    NetworkManager::NetworkManager()
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

    NetworkManager::~NetworkManager()
    {
        Disconnect();
        WSACleanup();
    }

    bool NetworkManager::Connect(const std::string& ip, int port, HWND hWnd)
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
        if (!m_socket.SetAsync(hWnd))
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

    void NetworkManager::Disconnect()
    {
        if (m_bConnected)
        {
            m_socket.Close();
            m_bConnected = false;
        }
    }

    void NetworkManager::SendLogin(const std::string& id, const std::string& pw)
    {
        std::vector<std::string> datas = { id, pw };
        SendPacket(PKT_LOGIN, datas);
    }

    void NetworkManager::SendRegister(const std::string& id, const std::string& pw)
    {
        std::vector<std::string> datas = { id, pw };
        SendPacket(PKT_REGISTER, datas);
    }

    void NetworkManager::SendChat(const std::string& msg)
    {
        std::vector<std::string> datas = { msg };
        SendPacket(PKT_CHAT, datas);
    }

    void NetworkManager::SendSelectCharacter(const std::string& charName)
    {
        std::vector<std::string> datas = { charName };
        SendPacket(PKT_SELECT_CHARACTER, datas);
    }

    void NetworkManager::SendSelectChannel(int channelId)
    {
        std::string channelStr = std::to_string(channelId);
        std::vector<std::string> datas = { channelStr };
        SendPacket(PKT_SELECT_CHANNEL, datas);
    }

    void NetworkManager::SendPacket(uint16_t type, const std::vector<std::string>& datas)
    {
        if (!m_bConnected)
        {
            return;
        }

        std::string body = PacketParser::MakeBody(datas);
        std::string packet = PacketParser::MakePacket(type, body);

        m_socket.SendPacket(packet);
    }

    void NetworkManager::ProcessReceivedData()
    {
        // 데이터 수신
        char tempBuf[BUFFER_SIZE];
        int recvLen = m_socket.Receive(tempBuf, BUFFER_SIZE);

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
                auto parsedPacket = PacketParser::Parse(recvBuffer);
                if (!parsedPacket.has_value())
                {
                    break;
                }

                // 로그 주석 처리
                // std::stringstream ss2;
                // ss2 << "[ProcessReceivedData] 패킷 파싱 완료: type=0x" << std::hex << parsedPacket.value().type << "\n";
                // OutputDebugStringA(ss2.str().c_str());
                
                HandlePacket(parsedPacket.value());
            }
        }
        else if (recvLen == 0)
        {
            // 연결 종료
            OutputDebugStringA("[ProcessReceivedData] 연결 종료\n");
            Disconnect();
        }
    }

    void NetworkManager::HandlePacket(const ParsedPacket& packet)
    {
        auto it = m_handlers.find(packet.type);
        if (it != m_handlers.end())
        {
            it->second(packet);
        }
    }

    void NetworkManager::RegisterHandler(uint16_t type, PacketHandler handler)
    {
        m_handlers[type] = handler;
    }

    void NetworkManager::UnregisterHandler(uint16_t type)
    {
        m_handlers.erase(type);
    }
}
