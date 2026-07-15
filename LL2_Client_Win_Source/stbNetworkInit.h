#pragma once
#include "stbNetworkManager.h"
#include "stbNetworkConfig.h"
#include "Packet.h"
#include <string>

namespace stb
{
    // 네트워크 초기화 헬퍼 함수
    inline void InitializeNetwork(HWND hWnd)
    {
        bool connected = NetworkManager::getInstance()->Connect(
            NetworkConfig::SERVER_IP,
            NetworkConfig::SERVER_PORT,
            hWnd
        );

        if (connected)
        {
            OutputDebugStringA("서버 연결 시도 중...\n");
        }
        else
        {
            OutputDebugStringA("서버 연결 실패!\n");
        }
    }

    // 플레이어 접속 패킷 전송 (패킷 타입 9, 데이터 "1")
    inline void SendPlayerConnect()
    {
        std::vector<std::string> data = { "1" };
        NetworkManager::getInstance()->SendPacket(PKT_PLAYER_CONNECT, data);
        OutputDebugStringA("플레이어 접속 패킷 전송 (타입: 9, 데이터: 1)\n");
    }

    // 로그인 핸들러 등록
    inline void RegisterLoginHandler()
    {
        NetworkManager::getInstance()->RegisterHandler(PKT_LOGIN, 
            [](const ParsedPacket& pkt) 
            {
                const char* data = pkt.payload.data();
                size_t offset = 0;

                uint16_t len = *(uint16_t*)(data + offset);
                offset += sizeof(uint16_t);

                std::string result(data + offset, len);

                if (result == "SUCCESS")
                {
                    OutputDebugStringA("로그인 성공!\n");
                }
                else
                {
                    std::string msg = "로그인 실패: " + result + "\n";
                    OutputDebugStringA(msg.c_str());
                }
            }
        );
    }

    // 채팅 핸들러 등록
    inline void RegisterChatHandler()
    {
        NetworkManager::getInstance()->RegisterHandler(PKT_CHAT,
            [](const ParsedPacket& pkt)
            {
                const char* data = pkt.payload.data();
                size_t offset = 0;

                uint16_t nameLen = *(uint16_t*)(data + offset);
                offset += sizeof(uint16_t);
                std::string userName(data + offset, nameLen);
                offset += nameLen;

                uint16_t msgLen = *(uint16_t*)(data + offset);
                offset += sizeof(uint16_t);
                std::string message(data + offset, msgLen);

                std::string debugMsg = "[채팅] " + userName + ": " + message + "\n";
                OutputDebugStringA(debugMsg.c_str());
            }
        );
    }

    // 플레이어 접속 응답 핸들러 등록
    inline void RegisterPlayerConnectHandler()
    {
        NetworkManager::getInstance()->RegisterHandler(PKT_PLAYER_CONNECT,
            [](const ParsedPacket& pkt)
            {
                OutputDebugStringA("플레이어 접속 응답 받음!\n");
                
                // 응답 데이터 파싱 (필요시)
                if (pkt.payload.size() > 0)
                {
                    const char* data = pkt.payload.data();
                    size_t offset = 0;

                    uint16_t len = *(uint16_t*)(data + offset);
                    offset += sizeof(uint16_t);

                    std::string response(data + offset, len);
                    std::string msg = "서버 응답: " + response + "\n";
                    OutputDebugStringA(msg.c_str());
                }
            }
        );
    }

    // 모든 핸들러 등록
    inline void RegisterAllHandlers()
    {
        RegisterLoginHandler();
        RegisterChatHandler();
        RegisterPlayerConnectHandler();
    }
}
