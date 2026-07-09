#pragma once
#include <afxsock.h>
#include <string>
#include "Packet.h"
// MySocket 명령 대상


class MySocket : public CSocket
{
public:
	HWND m_hWndOwner;
	BOOL m_bIsConnect;

	MySocket();
	virtual ~MySocket();

	
    virtual void OnReceive(int nErrorCode);
    virtual void OnClose(int nErrorCode);
};


