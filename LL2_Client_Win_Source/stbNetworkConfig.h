#pragma once
#include <string>

namespace stb
{
    // 전역 변수 선언
    extern char g_CharacterId[32];
    extern std::string g_CharacterName;
    extern int g_ChannelPort;
    extern int g_ChatPort;
    extern std::string g_ChannelTicket;

    // 네트워크 설정
    // 서버 IP와 포트를 여기서 변경하세요!
    struct NetworkConfig
    {
        // ========== 여기를 수정하세요! ==========
        
        // 외부 서버 IP 주소 (예시)
        static constexpr const char* SERVER_IP = "100.108.54.60";  // 실제 서버 IP로 변경!
        //static constexpr const char* SERVER_IP = "100.114.42.54";  // 실제 서버 IP로 변경!
       
        // 서버 포트
        //static constexpr int SERVER_PORT = 1234;  // 서버 포트에 맞게 변경
        static constexpr int SERVER_PORT = 9001;  // 서버 포트에 맞게 변경
        
        // 캐릭터 ID (명령줄 인자로 설정됨)
        static const char* GetCharacterId() { return g_CharacterId; }

        static void SetCharacterName(const std::string& name) { g_CharacterName = name; }
        static std::string GetCharacterName() { return g_CharacterName; }

        
        // 맵 ID
        static constexpr const char* MAP_ID = "100000000";
        
        // ==============================
        
        // 자동 연결 여부 (false로 하면 수동으로 연결해야 함)
        static constexpr bool AUTO_CONNECT = true;
        
        // 재연결 시도 횟수
        static constexpr int RECONNECT_ATTEMPTS = 3;

        static void SetChannelTicket(const std::string& ticket);
        static const std::string& GetChannelTicket();
        static void ClearChannelTicket();
    };
}

