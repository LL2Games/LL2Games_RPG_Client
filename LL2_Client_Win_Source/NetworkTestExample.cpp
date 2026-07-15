// 네트워크 테스트 예시 코드
// PlayScene이나 PlayerScript에서 사용하세요

#include "stbNetworkInit.h"
#include "stbApplication.h"

/*
// ========== 사용 방법 1: PlayScene에서 자동 접속 ==========

// stbPlayScene.cpp의 Initialize()에 추가:

void PlayScene::Initialize()
{
    // ... 기존 초기화 코드 ...
    
    // 네트워크 연결
    HWND hWnd = stb::Application::getInstance()->GetHWND();
    stb::InitializeNetwork(hWnd);
    stb::RegisterAllHandlers();
    
    // 플레이어 접속 패킷 전송 (타입 9, 데이터 "1")
    stb::SendPlayerConnect();
    
    Scene::Initialize();
}

// ========== 사용 방법 2: 키 입력으로 테스트 ==========

// stbPlayerScript.cpp의 Update()에 추가:

void PlayerScript::Update()
{
    // ... 기존 업데이트 코드 ...
    
    // C 키를 누르면 서버 연결
    if (Input::GetKeyDown(eKeyCode::C))
    {
        HWND hWnd = stb::Application::getInstance()->GetHWND();
        stb::InitializeNetwork(hWnd);
        stb::RegisterAllHandlers();
    }
    
    // P 키를 누르면 플레이어 접속 패킷 전송
    if (Input::GetKeyDown(eKeyCode::P))
    {
        stb::SendPlayerConnect();
    }
}

// ========== 사용 방법 3: 수동으로 커스텀 패킷 전송 ==========

// 어디서든 사용 가능:

// 연결
HWND hWnd = stb::Application::getInstance()->GetHWND();
stb::NetworkManager::getInstance()->Connect("127.0.0.1", 9000, hWnd);

// 패킷 타입 9, 데이터 "1" 전송
std::vector<std::string> data = { "1" };
stb::NetworkManager::getInstance()->SendPacket(0x09, data);

// 또는 간단하게
stb::SendPlayerConnect();

// ========== 디버그 출력 확인 ==========

Visual Studio의 출력 창(Output Window)에서 다음 메시지 확인:
- "서버 연결 시도 중..."
- "플레이어 접속 패킷 전송 (타입: 9, 데이터: 1)"
- "플레이어 접속 응답 받음!"

*/
