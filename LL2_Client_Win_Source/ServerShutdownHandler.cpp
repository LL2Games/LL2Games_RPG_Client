#include "ServerShutdownHandler.h"

#include "stbApplication.h"
#include <Windows.h>


void ServerShutdownHandler::HandleServerShutdown(const ParsedPacket& /*pkt*/)
{

    OutputDebugStringA("[PKT_SERVER_SHUTDOWN_NOTIFY] 서버 종료 알림 수신\n");

    HWND hWnd = stb::Application::getInstance()->GetHWND();

    if (hWnd == nullptr || !IsWindow(hWnd))
        return;

    // LOGIN, WORLD, CHANNEL에서 동시에 패킷이 올 경우 중복 표시 방지
    static std::atomic_bool noticeShown{ false };

    bool expected = false;

    if (!noticeShown.compare_exchange_strong(expected, true))
        return;

    MessageBoxW(
        hWnd,
        L"서버가 종료되었습니다.",
        L"서버 연결 종료",
        MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND
    );

    // 확인 버튼을 누르면 클라이언트 종료
    PostMessage(hWnd, WM_CLOSE, 0, 0);
}
