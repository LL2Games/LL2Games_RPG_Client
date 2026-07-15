// 네트워크 사용 예시 코드
// 이 파일은 참고용이며, 실제 Scene이나 Script에서 사용하시면 됩니다.

#include "stbNetworkManager.h"
#include "Packet.h"
#include <iostream>

namespace stb
{
    // 예시 1: 게임 시작 시 서버 연결 및 핸들러 등록
    void InitializeNetwork(HWND hWnd)
    {
        // 서버 연결
        bool connected = NetworkManager::getInstance()->Connect("127.0.0.1", 9000, hWnd);
        
        if (connected)
        {
            // 로그인 응답 핸들러
            NetworkManager::getInstance()->RegisterHandler(PKT_LOGIN, [](const ParsedPacket& pkt) {
                // payload에서 데이터 파싱
                const char* data = pkt.payload.data();
                size_t offset = 0;

                // 첫 번째 문자열 길이 읽기
                uint16_t len1 = *(uint16_t*)(data + offset);
                offset += sizeof(uint16_t);

                // 첫 번째 문자열 읽기 (예: 결과 코드)
                std::string result(data + offset, len1);
                offset += len1;

                if (result == "SUCCESS")
                {
                    // 로그인 성공 처리
                    std::cout << "로그인 성공!" << std::endl;
                }
                else
                {
                    // 로그인 실패 처리
                    std::cout << "로그인 실패: " << result << std::endl;
                }
            });

            // 회원가입 응답 핸들러
            NetworkManager::getInstance()->RegisterHandler(PKT_REGISTER, [](const ParsedPacket& pkt) {
                const char* data = pkt.payload.data();
                size_t offset = 0;

                uint16_t len = *(uint16_t*)(data + offset);
                offset += sizeof(uint16_t);

                std::string result(data + offset, len);
                
                if (result == "SUCCESS")
                {
                    std::cout << "회원가입 성공!" << std::endl;
                }
                else
                {
                    std::cout << "회원가입 실패: " << result << std::endl;
                }
            });

            // 채팅 메시지 핸들러
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

                std::cout << userName << ": " << message << std::endl;
            });

            // 월드 초기화 핸들러
            NetworkManager::getInstance()->RegisterHandler(PKT_INIT_WORLD, [](const ParsedPacket& pkt) {
                // 월드 데이터 파싱 및 초기화
                std::cout << "월드 초기화 데이터 수신" << std::endl;
            });
        }
    }

    // 예시 2: 로그인 시도
    void TryLogin(const std::string& id, const std::string& password)
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            NetworkManager::getInstance()->SendLogin(id, password);
        }
    }

    // 예시 3: 회원가입 시도
    void TryRegister(const std::string& id, const std::string& password)
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            NetworkManager::getInstance()->SendRegister(id, password);
        }
    }

    // 예시 4: 채팅 메시지 전송
    void SendChatMessage(const std::string& message)
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            NetworkManager::getInstance()->SendChat(message);
        }
    }

    // 예시 5: 캐릭터 선택
    void SelectCharacter(const std::string& characterName)
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            NetworkManager::getInstance()->SendSelectCharacter(characterName);
        }
    }

    // 예시 6: 채널 선택
    void SelectChannel(int channelId)
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            NetworkManager::getInstance()->SendSelectChannel(channelId);
        }
    }

    // 예시 7: 커스텀 패킷 전송
    void SendCustomPacket()
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            std::vector<std::string> data = { "custom_data1", "custom_data2" };
            NetworkManager::getInstance()->SendPacket(0x100, data);
        }
    }

    // 예시 8: Scene에서 사용하는 방법
    // stbPlayScene.cpp의 Initialize()에서 호출
    /*
    void PlayScene::Initialize()
    {
        // 네트워크 초기화
        InitializeNetwork(Application::GetInstance()->GetHwnd());
        
        // 자동 로그인 시도
        TryLogin("testuser", "testpass");
    }
    */

    // 예시 9: Script에서 사용하는 방법
    // stbPlayerScript.cpp의 Update()에서 호출
    /*
    void PlayerScript::Update()
    {
        // 엔터키를 누르면 채팅 전송
        if (Input::GetKeyDown(eKeyCode::Return))
        {
            SendChatMessage("Hello from player!");
        }
    }
    */
}
