#include "stbNetworkConfig.h"

namespace stb
{
    // 전역 변수로 캐릭터 ID 저장
    char g_CharacterId[32] = "1";  // 기본값 "1"
    std::string g_CharacterName;
    std::string g_ChannelTicket;
    int g_ChannelPort = 9001;
    int g_ChatPort = 9101;

    void NetworkConfig::SetChannelTicket(const std::string& ticket)
    {
        g_ChannelTicket = ticket;
    }

    const std::string& NetworkConfig::GetChannelTicket()
    {
        return g_ChannelTicket;
    }

    void NetworkConfig::ClearChannelTicket()
    {
        g_ChannelTicket.clear();
    }

}
