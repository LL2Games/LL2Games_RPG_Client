#include "MovePacketHandler.h"
#include "stbNetworkConfig.h"
#include "stbOtherPlayerManager.h"
#include "PacketParser.h"
#include "StringConvert.h"
#include "PacketData.h"
#include "stbNetworkManager.h"
#include "stbTransform.h"
#include "stbLogger.h"
#include "PlayerManager.h"

#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()


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
        std::size_t offset = 0;
        const std::size_t payloadSize = pkt.payload.size();
        const char* data = pkt.payload.c_str();

        std::string firstField;
        std::string errMsg;

        if (payloadSize < sizeof(std::uint16_t))
        {
            return;
        }

        // 첫 필드는 ok, nok 또는 이동한 플레이어 ID다.
        if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, firstField, errMsg))
        {
            M_LOGGER("이동 패킷 첫 필드 파싱 실패: %s",errMsg.c_str());
            return;
        }

        std::string debugMessage ="[PKT_PLAYER_MOVE 수신] firstField=[" + firstField +"]\n";
        OutputDebugStringA(debugMessage.c_str());

        // 서버가 보낸 정상 이동 응답
        if (firstField == "ok")
        {
            return;
        }

        // 서버가 이동을 거부한 경우 서버 좌표로 보정
        if (firstField == "nok")
        {
            OutputDebugStringA("[MOVE] entered NOK handler\n");
            std::string reason;
            float serverX = 0.0F;
            float serverY = 0.0F;

            if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, reason, errMsg))
            {
                OutputDebugStringA("[MOVE] failed to parse reason\n");
                return;
            }

            if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, serverX, errMsg) || !PacketParser::ParseNextFloatField(data, payloadSize, offset, serverY, errMsg))
            {
                OutputDebugStringA("[MOVE] failed to parse server position\n");
                return;
            }

            std::string positionLog ="[MOVE] parsed server position x=" + std::to_string(serverX) + " y=" + std::to_string(serverY) +"\n";
            OutputDebugStringA(positionLog.c_str());

            stb::Player* localPlayer = M_PLAYERMANAGER->GetLocalPlayer();

            if (localPlayer == nullptr)
            {
                OutputDebugStringA("[MOVE] local player is null\n");
                return;
            }

            stb::Transform* transform =
                localPlayer->GetComponent<stb::Transform>();

            if (transform == nullptr)
            {
                OutputDebugStringA("[MOVE] local transform is null\n");
                return;
            }

            const stb::math::Vector2 serverPosition{serverX,serverY};

            transform->SetPosition(serverPosition);

            if (localPlayer->GetPlayerLocation() != nullptr)
            {
                localPlayer->GetPlayerLocation()->pos = serverPosition;
            }

            OutputDebugStringA("[MOVE] correction applied\n");

            return;
        }

        // ok/nok가 아니면 다른 플레이어의 ID다.
        const std::string& playerId = firstField;

        if (playerId == stb::NetworkConfig::GetCharacterId())
        {
            return;
        }

        int state = 0;
        OtherPlayerMove otherPlayerMove{};

        if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, otherPlayerMove.xPos, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, otherPlayerMove.yPos, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextFloatField( data, payloadSize, offset, otherPlayerMove.speed, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, otherPlayerMove.dir, errMsg))
        {
            return;
        }

        if (!PacketParser::ParseNextIntField(data, payloadSize, offset, state, errMsg))
        {
            return;
        }

        otherPlayerMove.state = (state == 1) ? PlayerState::Walk : PlayerTypeUtil::IntToState(state);

        otherPlayerMove.playerId = playerId;

        auto otherPlayerManager = stb::OtherPlayerManager::getInstance();

        if (otherPlayerManager == nullptr)
        {
            return;
        }

        otherPlayerManager->HandleMovePacket(otherPlayerMove);
    }
    catch (const std::exception& exception)
    {
        M_LOGGER(
            "이동 패킷 처리 중 예외 발생: %s",
            exception.what()
        );
    }
    catch (...)
    {
        M_LOGGER("이동 패킷 처리 중 알 수 없는 예외 발생");
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
