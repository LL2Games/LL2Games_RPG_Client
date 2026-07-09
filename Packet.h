#pragma once

#define BUFFER_SIZE 1024
#define WM_SOCKET_RECEIVE (WM_USER + 100)
#define WM_SOCKET_DISCONNECT (WM_USER + 101)

#pragma pack(push, 1)
struct PacketHeader
{
    uint16_t length; // 전체 패킷 길이
    uint16_t type;   // 패킷 타입
};
#pragma pack(pop)

// 패킷타입 정의
enum PACKET_TYPE : uint16_t {
    PKT_LOGIN = 0x01,
    PKT_REGISTER = 0x02,
    PKT_CHAT_INIT,
    PKT_CHAT,
    PKT_INIT_WORLD,
    PKT_SELECT_CHARACTER,
    PKT_SELECT_CHANNEL,
};

struct ParsedPacket
{
    uint16_t type;
    std::string payload;
};