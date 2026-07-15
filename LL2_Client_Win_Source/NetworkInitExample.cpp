// PlayScene에서 네트워크 초기화 예시
// 이 코드를 stbPlayScene.cpp의 Initialize()에 추가하세요

/*
#include "stbNetworkManager.h"
#include "stbNetworkConfig.h"
#include "stbApplication.h"
#include "Packet.h"

void PlayScene::Initialize()
{
    // 기존 카메라 및 게임 오브젝트 초기화...
    GameObject* camera = object::Instantiate<GameObject>(enums::eLayerType::Particle, Vector2(0.0f, 0.0f));
    Camera* cameraComp = camera->AddComponent<Camera>();
    render::mainCamera = cameraComp;
    
    // ... 기존 플레이어, 루이지 생성 코드 ...
    
    // ========== 네트워크 초기화 ==========
    if (NetworkConfig::AUTO_CONNECT)
    {
        HWND hWnd = Application::getInstance()->GetHwnd();
        bool connected = NetworkManager::getInstance()->Connect(
            NetworkConfig::SERVER_IP, 
            NetworkConfig::SERVER_PORT, 
            hWnd
        );
        
        if (connected)
        {
            // 로그인 응답 핸들러
            NetworkManager::getInstance()->RegisterHandler(PKT_LOGIN, [](const ParsedPacket& pkt) {
                const char* data = pkt.payload.data();
                size_t offset = 0;
                
                uint16_t len = *(uint16_t*)(data + offset);
                offset += sizeof(uint16_t);
                
                std::string result(data + offset, len);
                
                if (result == "SUCCESS")
                {
                    // 로그인 성공
                    OutputDebugStringA("로그인 성공!\n");
                }
                else
                {
                    // 로그인 실패
                    OutputDebugStringA("로그인 실패\n");
                }
            });
            
            // 채팅 핸들러
            NetworkManager::getInstance()->RegisterHandler(PKT_CHAT, [](const ParsedPacket& pkt) {
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
                
                // 디버그 출력
                std::string debugMsg = userName + ": " + message + "\n";
                OutputDebugStringA(debugMsg.c_str());
            });
            
            // 자동 로그인 (선택사항)
            // NetworkManager::getInstance()->SendLogin("testuser", "testpass");
        }
        else
        {
            OutputDebugStringA("서버 연결 실패\n");
        }
    }
    // ====================================
    
    Scene::Initialize();
}
*/

// ========== IP 변경 방법 ==========
// 
// 1. stbNetworkConfig.h 파일 열기
// 2. SERVER_IP 값 변경:
//    - 로컬 테스트: "127.0.0.1"
//    - 같은 네트워크: "192.168.x.x"
//    - 외부 서버: "your.server.com" 또는 실제 IP
// 3. SERVER_PORT 값 변경 (필요시)
// 4. 재컴파일
//
// ==================================
