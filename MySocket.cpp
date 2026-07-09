// MySocket.cpp: 구현 파일
//

#include "pch.h"
#include "LL2Games_Tester.h"
#include "MySocket.h"
#include "CStatusLed.h"
#include <vector>


// MySocket

MySocket::MySocket()
{
    m_bIsConnect = FALSE;
}

MySocket::~MySocket()
{
}

void MySocket::OnReceive(int nErrorCode)
{

    char temp[BUFFER_SIZE];
    int tempLen = 0;
    std::string buf;
    do
    {
        memset(temp, 0x00, sizeof(temp));
        tempLen = Receive(temp, sizeof(temp));
        buf.append(temp, tempLen);
    } while (tempLen == BUFFER_SIZE);

    if (!buf.empty())
    {
        // 디버그용 출력
        CString msg;
        msg.Format(L"Recv %d bytes", buf.size());
        /*AfxMessageBox(msg);*/

        auto* recvData = new std::vector<char>(buf.begin(), buf.end());
        ::PostMessage(m_hWndOwner, WM_SOCKET_RECEIVE, recvData->size(), (LPARAM)recvData);
    }

    CSocket::OnReceive(nErrorCode);
}

void MySocket::OnClose(int nErrorCode)
{
    AfxMessageBox(L"Server disconnected");
    Close();
    m_bIsConnect = FALSE;
    ::PostMessage(m_hWndOwner, WM_SOCKET_DISCONNECT, (WPARAM)ConnectionState::DISCONNECTED, (LPARAM)0);
    CSocket::OnClose(nErrorCode);
}

// MySocket 멤버 함수
