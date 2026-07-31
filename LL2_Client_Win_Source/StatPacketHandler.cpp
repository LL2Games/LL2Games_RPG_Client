#include "StatPacketHandler.h"
#include "stbNetworkDebug.h"
#include "UIManager.h"

#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()

void StatPacketHandler::HandleStatResult(const ParsedPacket& pkt)
{
    try 
    {
        size_t offset = 0;
        const char* data = pkt.payload.c_str();
        size_t payloadSize = pkt.payload.size();

        std::string result;
        std::string errMsg;


        if (!PacketParser::ParseLengthPrefixedString(
            data,
            payloadSize,
            offset,
            result,
            errMsg))
        {
            OutputDebugStringA(("[PKT_STAT_UP] result parse failed: " + errMsg + "\n").c_str());
            M_UIMANAGER->FinishStatUpRequest();
            throw std::runtime_error(errMsg);
        }

        if (result == "nok")
        {
            OutputDebugStringA(("[PKT_STAT_UP] unexpected result: " + result + "\n").c_str());
            M_UIMANAGER->FinishStatUpRequest();
        }
        std::string serverError;

        if (offset < payloadSize)
        {
            if (!PacketParser::ParseLengthPrefixedString(
                data,
                payloadSize,
                offset,
                serverError,
                errMsg))
            {
                serverError = "failed to parse server error";
            }
        }

        OutputDebugStringA(("[PKT_STAT_UP] failed: " + serverError + "\n").c_str());

        M_UIMANAGER->FinishStatUpRequest();

    }
    catch (const std::exception& e)
    {
        OutputDebugStringA(e.what());
    }
    catch (...)
    {
        OutputDebugStringA("예상치 못한 에러 입니다.\n");
    }
}

void StatPacketHandler::SendStatUp(const std::string statType)
{
    std::vector<std::string> data;

    data.push_back(statType);
 
    stb::NetworkManager::getInstance()->SendPacket(PKT_STAT_UP, data);
    OutputDebugStringA("[PKT_STAT_UP 전송 완료]\n\n");
}
