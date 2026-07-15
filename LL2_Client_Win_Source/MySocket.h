#pragma once
#include <string>
#include <vector>
#include "Packet.h"

// Windows 타입 전방 선언
typedef unsigned __int64 SOCKET;
typedef struct HWND__* HWND;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKET)(~0)
#endif

// WinSock2 관련 경고 무시
#pragma warning(disable: 4996)

class MySocket
{
public:
    MySocket();
    ~MySocket();

    // 소켓 생성 및 연결
    bool Create();
    bool Connect(const char* ip, int port);
    void Close();

    // 패킷 송수신
    bool SendPacket(const std::string& packet);
    int Receive(char* buffer, int bufferSize);

    // 비동기 설정
    bool SetAsync(HWND hWnd);
    bool SetAsync(HWND hWnd, const int wMsg);
    
    std::vector<char>& GetRecvBuffer() { return m_recvBuffer; }
    HWND GetNotifyWindow() const { return m_hNotifyWnd; }
    void SetNotifyWindow(HWND hWnd) { m_hNotifyWnd = hWnd; }
    
    bool IsValid() const { return m_socket != INVALID_SOCKET; }

private:
    SOCKET m_socket;
    std::vector<char> m_recvBuffer;
    HWND m_hNotifyWnd;
};

