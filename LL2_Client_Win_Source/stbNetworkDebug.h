#pragma once
#include "stbNetworkManager.h"
#include "stbNetworkConfig.h"
#include "stbOtherPlayerManager.h"
#include "stbLogger.h"
#include "Packet.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "..\\LL2_Client_Win_Source\\stbLogger.h"


namespace stb
{
    // 함수 전방 선언
    inline void SendEnterMap(const std::string& charId, const std::string& mapId);

    // 패킷 데이터를 16진수로 출력하는 헬퍼 함수
    inline void PrintPacketHex(const std::string& packet, const char* label)
    {
        std::stringstream ss;
        ss << label << " (" << packet.size() << " bytes): ";
        
        for (size_t i = 0; i < packet.size(); i++)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << (int)(unsigned char)packet[i] << " ";
        }
        ss << "\n";
        
        OutputDebugStringA(ss.str().c_str());
    }

    // 디버그 모드로 네트워크 초기화
    inline void InitializeNetworkDebug(HWND hWnd)
    {
        std::stringstream ss;
        ss << "=== 네트워크 디버그 모드 ===\n";
        //ss << "서버 IP: " << NetworkConfig::SERVER_IP << "\n";
        //ss << "서버 포트: " << NetworkConfig::SERVER_PORT << "\n";
        ss << "서버 IP: " << g_ServerIp << "\n";
        ss << "서버 포트: " << g_ChannelPort << "\n";
        OutputDebugStringA(ss.str().c_str());

        bool connected = NetworkManager::getInstance()->Connect(
            g_ServerIp,
            //NetworkConfig::SERVER_IP,
            //NetworkConfig::SERVER_PORT,
            g_ChannelPort,
            //9001,
            hWnd
        );

        if (connected)
        {
            OutputDebugStringA("서버 연결 시도 중...\n");
        }
        else
        {
            OutputDebugStringA("서버 연결 실패! (소켓 생성 또는 connect 실패)\n");
        }
    }

    // 연결 상태 확인
    inline void CheckConnectionStatus()
    {
        if (NetworkManager::getInstance()->IsConnected())
        {
            OutputDebugStringA("현재 상태: 서버 연결됨\n");
        }
        else
        {
            OutputDebugStringA("현재 상태: 서버 연결 안 됨\n");
        }
    }


    // 채널 인증 패킷 전송 (패킷 타입 0x09, 데이터: 캐릭터 ID)
    inline void SendChannelAuth()
    {
        std::string charId = NetworkConfig::GetCharacterId();
        std::vector<std::string> data = { charId };
        
        std::stringstream ss;
        ss << "\n[PKT_CHANNEL_AUTH 전송]\n";
        ss << "  패킷 타입: 0x" << std::hex << PKT_CHANNEL_AUTH << " (" << std::dec << PKT_CHANNEL_AUTH << ")\n";
        ss << "  캐릭터 ID: " << charId << "\n";
        OutputDebugStringA(ss.str().c_str());
        try
        {
            // 패킷 생성 및 전송
            std::string body = PacketParser::MakeBody(data);
            std::string packet = PacketParser::MakePacket(PKT_CHANNEL_AUTH, body);

            // 패킷 내용 출력
            PrintPacketHex(packet, "  전송 패킷");
        }
        catch (const std::length_error& e)
        {
            M_LOGGER("CharacterList 패킷 크기 초과: %s", e.what());
            goto err;
        }
        catch (const std::exception& e)
        {
            M_LOGGER("CharacterList 패킷 생성 실패: %s", e.what());
            goto err;
        }
        
        NetworkManager::getInstance()->SendPacket(PKT_CHANNEL_AUTH, data);
        OutputDebugStringA("[PKT_CHANNEL_AUTH 전송 완료]\n\n");

    err:
        return;
    }

    // 맵 입장 패킷 전송 (캐릭터 ID, 맵 ID)
    inline void SendEnterMap(const std::string& charId, const std::string& mapId)
    {
        std::vector<std::string> data = { charId, mapId };
        
        std::stringstream ss;
        ss << "\n[PKT_ENTER_MAP 전송]\n";
        ss << "  패킷 타입: 0x" << std::hex << PKT_ENTER_MAP << " (" << std::dec << PKT_ENTER_MAP << ")\n";
        ss << "  캐릭터 ID: " << charId << "\n";
        ss << "  맵 ID: " << mapId << "\n";
        OutputDebugStringA(ss.str().c_str());
        try
        {
            // 패킷 생성 및 전송
            std::string body = PacketParser::MakeBody(data);
            std::string packet = PacketParser::MakePacket(PKT_ENTER_MAP, body);
            // 패킷 내용 출력
            PrintPacketHex(packet, "  전송 패킷");
        }
        catch (const std::length_error& e)
        {
            M_LOGGER("CharacterList 패킷 크기 초과: %s", e.what());
            goto err;
        }
        catch (const std::exception& e)
        {
            M_LOGGER("CharacterList 패킷 생성 실패: %s", e.what());
            goto err;
        }


         NetworkManager::getInstance()->SendPacket(PKT_ENTER_MAP, data);
         OutputDebugStringA("[PKT_ENTER_MAP 전송 완료]\n\n");
        
     err:
         return;
    }

    // 플레이어 이동 패킷 전송 (x, y, speed)
    inline void SendPlayerMove(float x, float y, float speed)
    {
        if (!NetworkManager::getInstance()->IsConnected())
        {
            return;  // 연결 안 되어 있으면 전송 안 함
        }

        char xStr[32], yStr[32], speedStr[32];
        sprintf_s(xStr, "%.1f", x);
        sprintf_s(yStr, "%.1f", y);
        sprintf_s(speedStr, "%.1f", speed);
      
        
        std::vector<std::string> data = { xStr, yStr, speedStr };
        NetworkManager::getInstance()->SendPacket(PKT_PLAYER_MOVE, data);
        
        // 디버그 로그 (너무 많이 나올 수 있으니 주석 처리 가능)
        // std::stringstream ss;
        // ss << "[전송] 이동 패킷: (" << x << ", " << y << ")\n";
        // OutputDebugStringA(ss.str().c_str());
    }
}
