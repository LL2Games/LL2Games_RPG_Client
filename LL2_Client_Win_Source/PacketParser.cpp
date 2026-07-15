#include "PacketParser.h"
#include "Packet.h"
#include "StringConvert.h"


std::string PacketParser::MakeBody(const std::vector<std::string>& datas)
{
    std::string body;

    for (auto& data : datas)
    {
        uint16_t dataLen = (uint16_t)data.size();
        body.append((char*)&dataLen, sizeof(dataLen));
        body.append(data);
    }

    return body;
}

std::string PacketParser::MakePacket(uint16_t type, const std::string& body)
{
    PacketHeader hdr;
    std::string packet;

    hdr.type = type;
    hdr.length = sizeof(PacketHeader) + body.size();

    packet.append((char*)&hdr, sizeof(hdr));
    packet.append(body);

    return packet;
}

std::optional<ParsedPacket> PacketParser::Parse(std::vector<char>& buf)
{
    ParsedPacket parsedPacket;
    if (buf.size() < sizeof(PacketHeader))
    {
        return std::nullopt;
    }

    PacketHeader* hdr = reinterpret_cast<PacketHeader*>(buf.data());
    uint16_t pktLen = hdr->length;

    if (buf.size() < pktLen)
    {
        return std::nullopt;
    }
    uint16_t type = hdr->type;

    const char* payload = reinterpret_cast<const char*>(buf.data() + sizeof(PacketHeader));
    int payloadLen = pktLen - sizeof(PacketHeader);

    parsedPacket.type = type;
    parsedPacket.payload = std::string(payload, payloadLen);

    buf.erase(buf.begin(), buf.begin() + pktLen);

    return parsedPacket;
}

bool PacketParser::ParseLengthPrefixedString(
    const char* payload,
    const size_t payload_len,
    size_t& offset,
    std::string& outValue,
    std::string& errMsg)
{
    if (payload == nullptr || payload_len == 0)
    {
        errMsg = "payload empty";
        return false;
    }

    if (offset >= payload_len)
    {
        errMsg = "offset overflow";
        return false;
    }

    // 1. length (1 byte)
    uint8_t value_len = static_cast<uint8_t>(payload[offset]);
    offset += 1;

    // 2. reserved byte skip
    if (offset < payload_len && payload[offset] == 0x00)
    {
        offset += 1;
    }

    // 3. bounds check
    if (offset + value_len > payload_len)
    {
        errMsg = "payload length overflow";
        return false;
    }

    // 4. extract value
    outValue.assign(payload + offset, value_len);
    offset += value_len;

    return true;
}

bool PacketParser::ParseNextIntField(const char* data, size_t payloadSize, size_t& offset, int& outValue, std::string& errMsg)
{
    std::string temp;

    if (!PacketParser::ParseLengthPrefixedString(
        data,
        payloadSize,
        offset,
        temp,
        errMsg))
    {
        return false;
    }

    if (!Convert::StringToInt(temp, outValue))
    {
        errMsg = "StringToInt failed: " + temp;
        return false;
    }

    return true;
}

bool PacketParser::ParseNextInt64Field(const char* data, size_t payloadSize, size_t& offset, int64_t& outValue, std::string& errMsg)
{
    std::string temp;

    if (!PacketParser::ParseLengthPrefixedString(
        data,
        payloadSize,
        offset,
        temp,
        errMsg))
    {
        return false;
    }

    OutputDebugStringA(temp.c_str());
    OutputDebugStringA("\n");

    if (!Convert::StringToInt64(temp, outValue))
    {
        errMsg = "StringTo64Int failed: " + temp;
        return false;
    }

    return true;
}

bool PacketParser::ParseNextFloatField(const char* data, size_t payloadSize, size_t& offset, float& outValue, std::string& errMsg)
{
    std::string temp;

    if (!PacketParser::ParseLengthPrefixedString(
        data,
        payloadSize,
        offset,
        temp,
        errMsg))
    {
        return false;
    }

    if (!Convert::StringToFloat(temp, outValue))
    {
        errMsg = "StringToFloat failed: " + temp;
        return false;
    }

    return true;
}
