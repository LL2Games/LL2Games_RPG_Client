#pragma once
#include "stbSingletonBase.h"
#include "MySocket.h"
#include "PacketParser.h"
#include <functional>
#include <map>
#include <string>

// Windows 타입 전방 선언
typedef struct HWND__* HWND;

namespace stb
{
    class ChatNetworkManager : public SingletonBase<ChatNetworkManager>
    {
    public:
        ChatNetworkManager();
        ~ChatNetworkManager();

        // 연결 관리
        bool Connect(const std::string& ip, int port, HWND hWnd);
        void Disconnect();
        bool IsConnected() const { return m_bConnected; }

        // 패킷 송신
        void SendChatInit(const std::string& char_id);
        void SendChat(const std::string& msg);

        // 커스텀 패킷 전송
        void SendPacket(uint16_t type, const std::vector<std::string>& datas);

        // 패킷 수신 처리
        void ProcessReceivedData();

        // 패킷 핸들러 등록
        using PacketHandler = std::function<void(const ParsedPacket&)>;
        void RegisterHandler(uint16_t type, PacketHandler handler);
        void UnregisterHandler(uint16_t type);

    private:
        MySocket m_socket;
        bool m_bConnected;
        std::map<uint16_t, PacketHandler> m_handlers;

        void HandlePacket(const ParsedPacket& packet);
    };
}
