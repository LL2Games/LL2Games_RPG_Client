# 네트워크 시스템 사용 가이드

## 개요
PacketParser와 MySocket을 기반으로 한 네트워크 통신 시스템입니다.
NetworkManager 싱글톤을 통해 서버와의 연결, 패킷 송수신, 핸들러 등록을 관리합니다.

## 파일 구조
- `MySocket.h/cpp` - CSocket 기반 비동기 소켓 클래스
- `stbNetworkManager.h/cpp` - 네트워크 관리 싱글톤
- `Packet.h` - 패킷 구조 및 타입 정의
- `PacketParser.h/cpp` - 패킷 생성 및 파싱

## 사용 방법

### 1. 서버 연결
```cpp
// PlayScene::Initialize() 또는 Application::Initialize()에서
#include "stbNetworkManager.h"
#include "stbApplication.h"

HWND hWnd = Application::getInstance()->GetHwnd();
bool connected = stb::NetworkManager::getInstance()->Connect("127.0.0.1", 9000, hWnd);

if (connected)
{
    // 연결 성공
}
```

### 2. 패킷 핸들러 등록
```cpp
// 로그인 응답 처리
stb::NetworkManager::getInstance()->RegisterHandler(PKT_LOGIN, [](const ParsedPacket& pkt) {
    const char* data = pkt.payload.data();
    size_t offset = 0;

    // 문자열 길이 읽기
    uint16_t len = *(uint16_t*)(data + offset);
    offset += sizeof(uint16_t);

    // 문자열 읽기
    std::string result(data + offset, len);
    offset += len;

    if (result == "SUCCESS")
    {
        // 로그인 성공 처리
    }
});

// 채팅 메시지 처리
stb::NetworkManager::getInstance()->RegisterHandler(PKT_CHAT, [](const ParsedPacket& pkt) {
    const char* data = pkt.payload.data();
    size_t offset = 0;

    // 유저 이름
    uint16_t nameLen = *(uint16_t*)(data + offset);
    offset += sizeof(uint16_t);
    std::string userName(data + offset, nameLen);
    offset += nameLen;

    // 메시지
    uint16_t msgLen = *(uint16_t*)(data + offset);
    offset += sizeof(uint16_t);
    std::string message(data + offset, msgLen);

    // 채팅 UI에 표시
});
```

### 3. 패킷 전송
```cpp
// 로그인
stb::NetworkManager::getInstance()->SendLogin("user123", "password");

// 회원가입
stb::NetworkManager::getInstance()->SendRegister("newuser", "newpass");

// 채팅
stb::NetworkManager::getInstance()->SendChat("Hello World!");

// 캐릭터 선택
stb::NetworkManager::getInstance()->SendSelectCharacter("Warrior");

// 채널 선택
stb::NetworkManager::getInstance()->SendSelectChannel(1);

// 커스텀 패킷
std::vector<std::string> data = { "data1", "data2", "data3" };
stb::NetworkManager::getInstance()->SendPacket(0x100, data);
```

### 4. 연결 상태 확인
```cpp
if (stb::NetworkManager::getInstance()->IsConnected())
{
    // 연결된 상태
}
```

### 5. 연결 종료
```cpp
stb::NetworkManager::getInstance()->Disconnect();
```

## 패킷 구조

### 패킷 헤더
```cpp
struct PacketHeader
{
    uint16_t length;  // 전체 패킷 길이 (헤더 + 바디)
    uint16_t type;    // 패킷 타입
};
```

### 패킷 바디
바디는 여러 개의 문자열로 구성되며, 각 문자열은 다음 형식:
```
[2바이트 길이][문자열 데이터]
```

### 패킷 타입
```cpp
enum PACKET_TYPE : uint16_t {
    PKT_LOGIN = 0x01,           // 로그인
    PKT_REGISTER = 0x02,        // 회원가입
    PKT_CHAT_INIT,              // 채팅 초기화
    PKT_CHAT,                   // 채팅 메시지
    PKT_INIT_WORLD,             // 월드 초기화
    PKT_SELECT_CHARACTER,       // 캐릭터 선택
    PKT_SELECT_CHANNEL,         // 채널 선택
};
```

## 윈도우 메시지 처리

LL2_Client_Win.cpp의 WndProc에 이미 추가되어 있습니다:
```cpp
case WM_SOCKET_RECEIVE:
    // 서버로부터 데이터 수신
    stb::NetworkManager::getInstance()->ProcessReceivedData();
    break;

case WM_SOCKET_DISCONNECT:
    // 서버 연결 끊김
    stb::NetworkManager::getInstance()->Disconnect();
    break;
```

## Scene에서 사용 예시

```cpp
// stbPlayScene.cpp
void PlayScene::Initialize()
{
    // 기존 초기화 코드...
    
    // 네트워크 연결
    HWND hWnd = Application::getInstance()->GetHwnd();
    stb::NetworkManager::getInstance()->Connect("127.0.0.1", 9000, hWnd);
    
    // 핸들러 등록
    stb::NetworkManager::getInstance()->RegisterHandler(PKT_LOGIN, [](const ParsedPacket& pkt) {
        // 로그인 응답 처리
    });
    
    Scene::Initialize();
}
```

## Script에서 사용 예시

```cpp
// stbPlayerScript.cpp
void PlayerScript::Update()
{
    // 엔터키로 채팅 전송
    if (Input::GetKeyDown(eKeyCode::Return))
    {
        stb::NetworkManager::getInstance()->SendChat("Hello!");
    }
    
    // 스페이스바로 캐릭터 선택
    if (Input::GetKeyDown(eKeyCode::Space))
    {
        stb::NetworkManager::getInstance()->SendSelectCharacter("Mario");
    }
}
```

## 주의사항

1. NetworkManager는 싱글톤이므로 GetInstance()로 접근
2. 패킷 전송 전에 IsConnected() 확인 권장
3. 핸들러는 Scene 초기화 시 한 번만 등록
4. payload 파싱 시 offset 관리 주의
5. 서버 프로토콜과 패킷 구조가 일치해야 함

## 새로운 패킷 타입 추가

1. Packet.h에 패킷 타입 추가
```cpp
enum PACKET_TYPE : uint16_t {
    // 기존 타입들...
    PKT_NEW_TYPE = 0x10,  // 새로운 타입
};
```

2. NetworkManager에 전송 함수 추가 (선택사항)
```cpp
void NetworkManager::SendNewPacket(const std::string& data)
{
    std::vector<std::string> datas = { data };
    SendPacket(PKT_NEW_TYPE, datas);
}
```

3. 핸들러 등록
```cpp
stb::NetworkManager::getInstance()->RegisterHandler(PKT_NEW_TYPE, [](const ParsedPacket& pkt) {
    // 처리 로직
});
```
