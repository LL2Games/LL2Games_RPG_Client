// WinAPIReview_Engine.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "LL2_Client_Win.h"
#include <Windows.h>
#include <cwchar>
#include <cstdio>
#include <fstream>

#include "..\\LL2_Client_Win_Source\\stbApplication.h"
#include "..\\LL2_Client_Win_lib\\stbLoadScenes.h"
#include "..\\LL2_Client_Win_lib\\stbLoadResource.h"
#include "..\\LL2_Client_Win_Source\\stbNetworkManager.h"
#include "..\\LL2_Client_Win_Source\\stbNetworkConfig.h"
#include "..\\LL2_Client_Win_Source\\stbNetworkDebug.h"
#include "..\\LL2_Client_Win_Source\\Packet.h"
#include "..\\LL2_Client_Win_Source\\\stbLogger.h"
#include "..\\LL2_Client_Win_Source\\\ChannelInitPacketHandler.h"

#include "..\\LL2_Client_Win_Source\\\stbChatNetworkManager.h"
#include "..\\LL2_Client_Win_Source\\\ChatPacketHandler.h"

#include <afxwin.h>
#include "CLogin.h"
#include "CWorld.h"
#include "MySocket.h"
#include "..\\LL2_Client_Win_Source\\\UIManager.h"

#define APP stb::SingletonBase<stb::Application>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()

ULONG_PTR gpToken;
Gdiplus::GdiplusStartupInput gdiplus;

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//로그인 아이디
std::string g_account_id;
//채널 포트
std::string g_channel_port;
//로그인 캐릭터 아이디
std::string g_char_id;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다

    //MFC초기화
    if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
    {
        MessageBoxA(NULL, "MFC 초기화 실패", "Error", MB_OK);
        return FALSE;
    }
    
    

    if (!AfxSocketInit())
    {
        MessageBoxA(NULL, "Socket 초기화 실패", "Error", MB_OK);
        return FALSE;
    }

#if 0 /*gunoo22 260518 테스트*/
    //로그인
    CLogin logDlg;
    if (logDlg.DoModal() != IDOK)
    {
        return FALSE;
    }
#endif
#if 0
    //캐릭터, 채널 선택
    CWorld worldDlg;
    if (worldDlg.DoModal() != IDOK)
    {
        return FALSE;
    }
    //test
    {
        CString strCharId, strChannelPort;
        strCharId = CString(g_char_id.c_str());
        strChannelPort = CString(g_channel_port.c_str());


        CString strTmp;
        strTmp.Format(_T("캐릭터[%s] 채널port[%s]"), strCharId, strChannelPort);
        AfxMessageBox(strTmp);
    }

    strcpy(stb::g_CharacterId, g_char_id.c_str());
    stb::g_ChannelPort = atoi(g_channel_port.c_str());


    //채팅서버 포트 = 채널서버 + 100 ex) channelport=9001 -> chatport = 9101
    stb::g_ChatPort = stb::g_ChannelPort + 100;
#else
    g_char_id = "1";
    g_channel_port = "9001";

    strcpy(stb::g_CharacterId, g_char_id.c_str());
    stb::g_ChannelPort = atoi(g_channel_port.c_str());
    //채팅서버 포트 = 채널서버 + 100 ex) channelport=9001 -> chatport = 9101
    stb::g_ChatPort = stb::g_ChannelPort + 100;

    //// 명령줄 인자로 캐릭터 ID 설정
    if (lpCmdLine && wcslen(lpCmdLine) > 0)
    {
      

        // 유니코드를 멀티바이트로 변환
        WideCharToMultiByte(CP_UTF8, 0, lpCmdLine, -1, stb::g_CharacterId, sizeof(stb::g_CharacterId), NULL, NULL);
        
        char msg[128];
        sprintf_s(msg, "캐릭터 ID 설정: %s\n", stb::g_CharacterId);
        OutputDebugStringA(msg);
        LOG("%s\n", msg);
    }
    else
    {
        OutputDebugStringA("캐릭터 ID: 1 (기본값)\n");
    }
#endif

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPITESTCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPITESTCLIENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
            // 게임 로직 실행 구역
        {
           APP->Run();
        }
    }

    Gdiplus::GdiplusShutdown(gpToken);
    return (int)msg.wParam;
}


//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_WINAPITESTCLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    const UINT width = 1366;
    const UINT height = 768;

    //#define CreateWindowW(lpClassName, lpWindowName, dwStyle, x, y,\
    //nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)\

//gunoo22 260710 개발시 전체 창 투명도 설정
#ifdef __DEV_OPACITY
    HWND hWnd = CreateWindowExW
    (
        WS_EX_LAYERED,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        width,
        height,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );

    if (!hWnd)
    {
        return FALSE;
    }

    //투명도 지정 0(완전투명) ~ 255(완전 불투명)
    BYTE opacity = 22;
    SetLayeredWindowAttributes(hWnd, 0, opacity, LWA_ALPHA);

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#else
    HWND hWnd = CreateWindowW
    (
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        width,
        height,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
#endif

    //APP초기화 실패시 구동 취소
    if (!APP->Initialize(hWnd, width, height))
    {
        return FALSE;
    }

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    Gdiplus::GdiplusStartup(&gpToken, &gdiplus, NULL);

    // 생성한 씬 Load
    stb::LoadResource();
    stb::LoadScene();


    return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_SOCKET_RECEIVE:
    {
        // 소켓 이벤트 확인
        int event = WSAGETSELECTEVENT(lParam);
        int error = WSAGETSELECTERROR(lParam);

        if (event == FD_READ && error == 0)
        {
            // 서버로부터 데이터 수신
            // OutputDebugStringA("[FD_READ] 데이터 수신 중...\n"); // 로그 주석 처리
            stb::NetworkManager::getInstance()->ProcessReceivedData();
        }
        else if (event == FD_CLOSE)
        {
            // 서버 연결 끊김
            stb::NetworkManager::getInstance()->Disconnect();
        }
        else if (event == FD_CONNECT)
        {
            if (error == 0)
            {
                // 연결 성공 - 채널 인증 패킷 전송
                OutputDebugStringA("서버 연결 성공! 채널 인증 시작...\n");
                ChannelInitPacketHandler::SendChannelAuth();
                //stb::SendChannelAuth();
            }
            else
            {
                // 연결 실패
                OutputDebugStringA("서버 연결 실패!\n");
                stb::NetworkManager::getInstance()->Disconnect();
            }
        }
    }
    break;
    case WM_CHAT_SOCKET_RECEIVE:
    {
        int event = WSAGETSELECTEVENT(lParam);
        int error = WSAGETSELECTERROR(lParam);

        if (event == FD_READ && error == 0)
            stb::ChatNetworkManager::getInstance()->ProcessReceivedData();
        else if (event == FD_CLOSE)
            stb::ChatNetworkManager::getInstance()->Disconnect();
        else if (event == FD_CONNECT && error == 0)
        {
            if (error == 0)
            {
                // 연결 성공 - 채널 인증 패킷 전송
                OutputDebugStringA("채팅 서버 연결 성공! 채팅서버 초기화 시작\n");
                ChatPacketHandler::SendChatInit(stb::NetworkConfig::GetCharacterId());
            }
            else
            {
                // 연결 실패
                OutputDebugStringA("채팅 서버 연결 실패!\n");
                stb::ChatNetworkManager::getInstance()->Disconnect();
            }
        }
            
    }
    break;
    case WM_CHAR:
    {

#if 1
        wchar_t ch = static_cast<wchar_t>(wParam);

        if (M_UIMANAGER->IsTradeQuantityInputActive())
        {
            M_UIMANAGER->AppendTradeQuantityChar(ch);
            break;
        }

        // Enter는 ChatScene::Update 또는 Trade OnKeyDown에서 처리
        if (ch == VK_RETURN)
            break;

        if (ch == VK_BACK)
        {
            if (M_UIMANAGER->IsInputFocused())
            {
                M_UIMANAGER->HandleBackspace();
            }
            else
            {
                M_UIMANAGER->AppendInputChar_Trade(ch);
            }

            break;
        }

        if (ch == 0x1B)
        {
            if (M_UIMANAGER->IsInputFocused())
            {
                M_UIMANAGER->ToggleChatInput();
            }
            else
            {
                M_UIMANAGER->CloseReqTradeUI();
            }

            break;
        }

        if (ch >= 0x20)
        {
            if (M_UIMANAGER->IsInputFocused())
            {
                M_UIMANAGER->AppendInputChar(ch);
            }
            else
            {
                M_UIMANAGER->AppendInputChar_Trade(ch);
            }
        }

#else
        wchar_t ch = (wchar_t)wParam;

        //채팅 입력중일 경우
        if (M_UIMANAGER->IsInputFocused())
        {
            if (ch == VK_RETURN)            // Enter는 ChatScene::Update에서 처리
                break;

            if (ch == VK_BACK)             // Backspace
            {
                M_UIMANAGER->HandleBackspace();
                break;
            }

            if (ch == 0x1B)                // ESC → 입력 모드 종료
            {
                M_UIMANAGER->ToggleChatInput();
                break;
            }

            if (ch >= 0x20)                // 출력 가능한 문자만 추가
            {
                M_UIMANAGER->AppendInputChar(ch); //채팅
                break;
            }
            break;
        }

        // 채팅 입력 중이 아닐 때만 교환신청 입력 처리
        if (ch == VK_BACK)
        {
            M_UIMANAGER->HandleBackspace_Trade();
            break;
        }

        if (ch == 0x1B)
        {
            M_UIMANAGER->CloseReqTradeUI();
            break;
        }

        if (ch >= 0x20)
        {
            M_UIMANAGER->AppendInputChar_Trade(ch);
        }
#endif
    }
    break;
    case WM_KEYDOWN:
    {
//         if (M_UIMANAGER->IsTradeQuantityInputActive())
//         {
//             M_UIMANAGER->KeyDownTradeQuantity(wParam);
//             break;
//         }

//         M_UIMANAGER->KeyDownTrade(wParam); //교환신청
        if (!M_UIMANAGER->IsInputFocused()) //채팅중이 아닐경우에만
            M_UIMANAGER->KeyDownTrade(wParam); //교환신청
    }
    break;
    case WM_DESTROY:
        stb::NetworkManager::getInstance()->Disconnect();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
