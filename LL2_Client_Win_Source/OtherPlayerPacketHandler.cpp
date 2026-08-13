#include "OtherPlayerPacketHandler.h"
#include "PacketParser.h"
#include "stbOtherPlayerManager.h"

#define OTHERPLAYERMANAGER stb::singletonBase<stb::OtherPlayerManager>::getInstance()

void OtherPlayerPacketHandler::HandleOtherPlayerEnter(const ParsedPacket& pkt)
{
	/*
	payload.push_back(std::to_string(player->GetId()));
    payload.push_back(std::to_string(player->GetJob()));  
    payload.push_back(std::to_string(player->GetPos().xPos));
    payload.push_back(std::to_string(player->GetPos().yPos));
    payload.push_back(std::to_string(player->GetDir()));
    payload.push_back(std::to_string(static_cast<int>(player->GetState())));
	*/

    try
    {
        size_t offset = 0;
        size_t payloadSize = pkt.payload.size();

        std::string errMsg;

        int state = 0;
        OtherPlayerInfo otherPlayerInfo{};
        
       

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.char_id, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseLengthPrefixedString(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.name, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.job, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.xPos, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.yPos, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.dir, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, state, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        otherPlayerInfo.state = PlayerTypeUtil::IntToState(state);

        std::string msg =
            "[OtherEnter] charId=" + std::to_string(otherPlayerInfo.char_id) +
            " name=" + otherPlayerInfo.name + 
            " job=" + std::to_string(otherPlayerInfo.job) +
            " dir=" + std::to_string(otherPlayerInfo.dir) +
            " state=" + std::to_string(state) +
            "\n";
        OutputDebugStringA(msg.c_str());

        auto otherPlayerMgr = stb::OtherPlayerManager::getInstance();

        if (otherPlayerMgr != nullptr)
        {
            otherPlayerMgr->AddPlayer(otherPlayerInfo);
        }
        else
        {
            throw std::runtime_error("AddPlayer Failed");
        }
    }
    catch(const std::exception& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러입니다.\n\n");
    }

}

void OtherPlayerPacketHandler::HandleOtherPlayerLeave(const ParsedPacket& pkt)
{
    try
    {
        std::size_t offset = 0;
        std::string errMsg;
        int playerId = 0;

        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), pkt.payload.size(), offset, playerId, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        auto* otherPlayerManager = stb::OtherPlayerManager::getInstance();

        if (otherPlayerManager == nullptr)
        {
            throw std::runtime_error("OtherPlayerManager is not available");
        }

        otherPlayerManager->RemovePlayer(std::to_string(playerId));

        const std::string message = "[OtherLeave] playerId=" +std::to_string(playerId) + "\n";
        OutputDebugStringA(message.c_str());
    }
    catch (const std::exception& exception)
    {
        OutputDebugStringA("[OtherLeave] packet handling failed: ");
        OutputDebugStringA(exception.what());
        OutputDebugStringA("\n");
    }
}

void OtherPlayerPacketHandler::HandleOtherPlayerSnapShot(const ParsedPacket& pkt)
{
    try
    {
        size_t offset = 0;
        size_t payloadSize = pkt.payload.size();

        std::string errMsg;
        int size = 0;
        int state = 0;
       
        if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, size, errMsg))
        {
            throw std::runtime_error(errMsg);
        }

        for (int i = 0; i < size; i++)
        {
            OtherPlayerInfo otherPlayerInfo{};
            if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.char_id, errMsg))
            {
                throw std::runtime_error(errMsg);
            }


            if (!PacketParser::ParseLengthPrefixedString(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.name, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.job, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.xPos, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.yPos, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, otherPlayerInfo.dir, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            if (!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, state, errMsg))
            {
                throw std::runtime_error(errMsg);
            }

            otherPlayerInfo.state = PlayerTypeUtil::IntToState(state);

            std::string msg =
                "[SnapShot] charId=" + std::to_string(otherPlayerInfo.char_id) +
                " name=" + otherPlayerInfo.name +
                " job=" + std::to_string(otherPlayerInfo.job) +
                " dir=" + std::to_string(otherPlayerInfo.dir) +
                " state=" + std::to_string(state) +
                "\n";
            OutputDebugStringA(msg.c_str());

            auto otherPlayerMgr = stb::OtherPlayerManager::getInstance();

            if (otherPlayerMgr != nullptr)
            {
                otherPlayerMgr->AddPlayer(otherPlayerInfo);
            }
            else
            {
                throw std::runtime_error("AddPlayer Failed");
            }
        }

       
    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러입니다.\n\n");
    }
}
