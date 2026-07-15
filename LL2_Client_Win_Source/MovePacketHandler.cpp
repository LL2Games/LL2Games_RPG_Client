#include "MovePacketHandler.h"
#include "stbNetworkConfig.h"
#include "stbOtherPlayerManager.h"
#include "PacketParser.h"
#include "StringConvert.h"
#include "PacketData.h"
#include "stbNetworkManager.h"
#include "stbTransform.h"


/*
struct ParsedPacket
{
    uint16_t type;
    std::string payload;
};
*/



void MovePacketHandler::Execute(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        size_t payloadSize = pkt.payload.size();

        std::string playerId;
        std::string errMsg;
        int state = 0;
        OtherPlayerMove otherPlayerMove{};

        if (payloadSize < sizeof(uint16_t))
        {
            //LOG("[이동 패킷] 페이로드 크기 부족\n");
            return;
        }

        // 1. playerID (char_id)
        if (!PacketParser::ParseLengthPrefixedString(
            pkt.payload.c_str(),
            payloadSize,
            offset,
            playerId,
            errMsg
        ))
        {
            // 로그 출력 필요
            return;
        }

        if (playerId == stb::NetworkConfig::GetCharacterId())
        {
            return;
        }

        if(!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerMove.xPos, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerMove.yPos, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerMove.speed, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerMove.dir, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, state, errMsg))
        {
            return;
        }

        otherPlayerMove.state = (state == 1) ? PlayerState::Walk : PlayerTypeUtil::IntToState(state);
        otherPlayerMove.playerId = playerId;

        auto otherPlayerMgr = stb::OtherPlayerManager::getInstance();

        if (otherPlayerMgr != nullptr)
        { 
            otherPlayerMgr->HandleMovePacket(otherPlayerMove);
        }
        else
        {
            // 로그 출력 필요
            return;
        }
    }
    catch (const std::exception& e)
    {
        std::string msg = "[이동 패킷] 예외 발생: ";
        msg += e.what();
        msg += "\n";
        //LOG(msg);
    }
    catch (...)
    {
        //LOG("[이동 패킷] 알 수 없는 예외 발생\n");
    }
    
}

void MovePacketHandler::SendPlayerMove(stb::Player* player)
{
    if (player == nullptr)
        return;

    stb::Transform* tr = player->GetComponent<stb::Transform>();
    if (tr == nullptr)
        return;

    stb::math::Vector2 pos = tr->GetPosition();
    if (player->GetPlayerLocation() != nullptr)
    {
        player->GetPlayerLocation()->pos = pos;
    }

    std::vector<std::string> payload;

    payload.push_back(std::to_string(pos.x));
    payload.push_back(std::to_string(pos.y));
    payload.push_back(std::to_string(player->GetPlayerMoveSpeed()));
    payload.push_back(std::to_string(static_cast<int>(player->GetFacing())));
    payload.push_back(std::to_string(static_cast<int>(player->GetState())));
    
    stb::NetworkManager::getInstance()->SendPacket(PKT_PLAYER_MOVE, payload);
    OutputDebugStringA("[PKT_PLAYER_MOVE 전송 완료]\n\n");
}
