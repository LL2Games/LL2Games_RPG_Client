#include "ChannelInitPacketHandler.h"
#include "stbNetworkManager.h"
#include "stbNetworkConfig.h"
#include "Packet.h"
#include <sstream>
#include <algorithm>
// 태스트를 위해서 임시로 추가
#include <Windows.h>

#include "..\\LL2_Client_Win_Source\\stbLogger.h"

void ChannelInitPacketHandler::Execute(const ParsedPacket& pkt)
{
    stb::NetworkConfig::ClearChannelTicket();

    try
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();
        std::string status, name, errMsg;

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, status, errMsg))
        {
            return;
        }
           
        if (status != "ok")
        {
            std::string serverError;

            if (offset < payloadSize)
            {
                PacketParser::ParseLengthPrefixedString(data, payloadSize,offset,serverError,errMsg);
            }

            M_LOGGER("Channel authentication failed");

            ::MessageBoxW(
                nullptr,
                L"Channel 인증에 실패했습니다. 다시 로그인해 주세요.",
                L"인증 실패",
                MB_OK | MB_ICONERROR
            );
            return;
        }

        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, name, errMsg))
            return;

        if (offset != payloadSize)
        {
            M_LOGGER("Channel authentication response contains trailing data");
            return;
        }

        stb::NetworkConfig::SetCharacterName(name); 
        // 채널 인증 성공 후 맵 입장 패킷 전송
        OutputDebugStringA("채널 인증 완료! 맵 입장 패킷 전송...\n");
        SendEnterMap();
    }
    catch (...)
    {
        OutputDebugStringA("채널 인증 응답 파싱 중 오류\n");
    }
}

void ChannelInitPacketHandler::SendChannelAuth()
{
    const std::string& channelTicket = stb::NetworkConfig::GetChannelTicket();

    const bool isValidTicket = channelTicket.size() == 64 &&
        std::all_of(
            channelTicket.begin(),
            channelTicket.end(),
            [](const char value)
            {
                return
                    (value >= '0' && value <= '9') ||
                    (value >= 'a' && value <= 'f');
            }
        );

    if (!isValidTicket)
    {
        M_LOGGER("Channel authentication ticket is missing or invalid");
        return;
    }

    std::vector<std::string> data = {channelTicket};

    std::stringstream ss;
    ss << "\n[PKT_CHANNEL_AUTH 전송]\n";
    ss << "  패킷 타입: 0x" << std::hex << PKT_CHANNEL_AUTH << " (" << std::dec << PKT_CHANNEL_AUTH << ")\n";
    OutputDebugStringA(ss.str().c_str());

    try
    {
        stb::NetworkManager::getInstance()->SendPacket(PKT_CHANNEL_AUTH, data);
        OutputDebugStringA("[PKT_CHANNEL_AUTH 전송 완료]\n\n");
    }
    catch (const std::length_error& e)
    {
        M_LOGGER("CharacterList 패킷 크기 초과: %s", e.what());
    }
    catch (const std::exception& e)
    {
        M_LOGGER("CharacterList 패킷 생성 실패: %s", e.what());
    }
}

void ChannelInitPacketHandler::SendEnterMap()
{
    const std::vector<std::string> data;

    try
    {
        stb::NetworkManager::getInstance()->SendPacket(PKT_ENTER_MAP,data);
        OutputDebugStringA("[PKT_ENTER_MAP 전송 완료]\n");
    }
    catch (const std::length_error& exception)
    {
        M_LOGGER("EnterMap 패킷 크기 초과: %s",exception.what());
    }
    catch (const std::exception& exception)
    {
        M_LOGGER("EnterMap 패킷 생성 실패: %s",exception.what());
    }
}
