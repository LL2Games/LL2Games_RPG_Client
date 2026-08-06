#include "MySocket.h"
#include "Packet.h"
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

MySocket::MySocket()
    : m_socket(INVALID_SOCKET)
    , m_hNotifyWnd(nullptr)
{
    m_recvBuffer.reserve(PacketLimits::kMaxPacketSize);
}

MySocket::~MySocket()
{
    Close();
}

bool MySocket::Create()
{
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return m_socket != INVALID_SOCKET;
}

bool MySocket::Connect(const char* ip, int port)
{
    if (m_socket == INVALID_SOCKET)
    {
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(static_cast<u_short>(port));
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    int result = connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    
    if (result == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK)
        {
            return false;
        }
    }

    return true;
}

void MySocket::Close()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

bool MySocket::SetAsync(HWND hWnd)
{
    m_hNotifyWnd = hWnd;

    // 비동기 소켓 설정
    int result = WSAAsyncSelect(m_socket, hWnd, WM_SOCKET_RECEIVE, FD_READ | FD_CLOSE | FD_CONNECT);

    return result != SOCKET_ERROR;
}

bool MySocket::SetAsync(HWND hWnd, const int wMsg)
{
    m_hNotifyWnd = hWnd;

    // 비동기 소켓 설정
    int result = WSAAsyncSelect(m_socket, hWnd, wMsg, FD_READ | FD_CLOSE | FD_CONNECT);

    return result != SOCKET_ERROR;
}

int MySocket::Receive(char* buffer, int bufferSize)
{
    return recv(m_socket, buffer, bufferSize, 0);
}

bool MySocket::SendPacket(const std::string& packet)
{
    int totalSent = 0;
    int packetSize = (int)packet.size();

    while (totalSent < packetSize)
    {
        int sent = send(m_socket, packet.data() + totalSent, packetSize - totalSent, 0);
        if (sent == SOCKET_ERROR)
        {
            return false;
        }
        totalSent += sent;
    }

    return true;
}
