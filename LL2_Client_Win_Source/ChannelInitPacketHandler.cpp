#include "ChannelInitPacketHandler.h"
#include "stbNetworkManager.h"
#include "stbNetworkConfig.h"
#include "Packet.h"
#include <sstream>

void ChannelInitPacketHandler::Execute(const ParsedPacket& pkt)
{
    try
    {
#if 0
        size_t offset = 0;
        size_t payloadSize = pkt.payload.size();
        std::string debugMsg = "패킷사이즈" + payloadSize;
        OutputDebugStringA(debugMsg.c_str());

        if (payloadSize < sizeof(uint16_t))
        {
            //LOG("[이동 패킷] 페이로드 크기 부족\n");
            return;
        }
#else
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string status, name, errMsg;

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, errMsg))
            return;

        if (status == "nok")
            return;

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, name, errMsg))
            return;

        stb::NetworkConfig::SetCharacterName(name);
#endif
    
        // 채널 인증 성공 후 맵 입장 패킷 전송
        OutputDebugStringA("채널 인증 완료! 맵 입장 패킷 전송...\n");
         
        SendEnterMap(stb::NetworkConfig::GetCharacterId(), stb::NetworkConfig::MAP_ID);
    }
    catch (...)
    {
        OutputDebugStringA("채널 인증 응답 파싱 중 오류\n");
    }
}

void ChannelInitPacketHandler::SendChannelAuth()
{
    std::string charId = stb::NetworkConfig::GetCharacterId();
    std::vector<std::string> data = { charId };

    std::stringstream ss;
    ss << "\n[PKT_CHANNEL_AUTH 전송]\n";
    ss << "  패킷 타입: 0x" << std::hex << PKT_CHANNEL_AUTH << " (" << std::dec << PKT_CHANNEL_AUTH << ")\n";
    ss << "  캐릭터 ID: " << charId << "\n";
    OutputDebugStringA(ss.str().c_str());

    // 패킷 생성 및 전송
    std::string body = PacketParser::MakeBody(data);
    std::string packet = PacketParser::MakePacket(PKT_CHANNEL_AUTH, body);

    stb::NetworkManager::getInstance()->SendPacket(PKT_CHANNEL_AUTH, data);
    OutputDebugStringA("[PKT_CHANNEL_AUTH 전송 완료]\n\n");
}

void ChannelInitPacketHandler::SendEnterMap(const std::string& charId, const std::string& mapId)
{
    std::vector<std::string> data = { charId, mapId };

    std::stringstream ss;
    ss << "\n[PKT_ENTER_MAP 전송]\n";
    ss << "  패킷 타입: 0x" << std::hex << PKT_ENTER_MAP << " (" << std::dec << PKT_ENTER_MAP << ")\n";
    ss << "  캐릭터 ID: " << charId << "\n";
    ss << "  맵 ID: " << mapId << "\n";
    OutputDebugStringA(ss.str().c_str());

    // 패킷 생성 및 전송
    std::string body = PacketParser::MakeBody(data);
    std::string packet = PacketParser::MakePacket(PKT_ENTER_MAP, body);

    // 패킷 내용 출력
    

    stb::NetworkManager::getInstance()->SendPacket(PKT_ENTER_MAP, data);
    OutputDebugStringA("[PKT_ENTER_MAP 전송 완료]\n\n");
}