#include "PacketParser.h"
#include "Packet.h"
#include "StringConvert.h"
#include <limits>
#include <stdexcept>

std::string PacketParser::MakeBody(const std::vector<std::string>& datas)
{
    std::string body;

    for (const auto& data : datas)
    {
        if (data.size() > (std::numeric_limits<uint16_t>::max)())
        {
            throw std::length_error("Packet field is too large");
        }

        const uint16_t dataLength = static_cast<uint16_t>(data.size());

        body.append(reinterpret_cast<const char*>(&dataLength),sizeof(dataLength));

        body.append(data);
    }


    return body;
}

std::string PacketParser::MakePacket(
    uint16_t type,
    const std::string& body)
{
    constexpr std::size_t headerSize = sizeof(PacketHeader);

    static_assert(
        PacketLimits::kMaxPacketSize >= headerSize,
        "Maximum packet size is smaller than packet header"
        );

    if (body.size() > PacketLimits::kMaxPacketSize - headerSize)
    {
        throw std::length_error("Packet body is too large");
    }

    const std::size_t packetLength = headerSize + body.size();

    PacketHeader header{};
    header.type = type;
    header.length = static_cast<uint16_t>(packetLength);

    std::string packet;
    packet.reserve(packetLength);

    packet.append(
        reinterpret_cast<const char*>(&header),
        sizeof(header)
    );

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

ParseResult PacketParser::TryParse(std::vector<char>& buf)
{
    if (buf.size() < sizeof(PacketHeader))
    {
        return { ParseStatus::NeedMoreData, {} };
    }

    PacketHeader header{};
    std::memcpy(&header, buf.data(), sizeof(header));

    const uint16_t packetLength = header.length;

    if (packetLength < sizeof(PacketHeader))
    {
        return { ParseStatus::InvalidPacket, {} };
    }

    if (packetLength > PacketLimits::kMaxPacketSize)
    {
        return { ParseStatus::InvalidPacket, {} };
    }

    if (buf.size() < packetLength)
    {
        return { ParseStatus::NeedMoreData, {} };
    }

    ParsedPacket parsedPacket{};
    parsedPacket.type = header.type;

    const char* payload = buf.data() + sizeof(PacketHeader);
    const std::size_t payloadLength =packetLength - sizeof(PacketHeader);

    parsedPacket.payload.assign(payload, payloadLength);

    buf.erase(buf.begin(),buf.begin() + packetLength);

    return {
        ParseStatus::Complete,
        std::move(parsedPacket)
    };
}

bool PacketParser::ParseLengthPrefixedString(
    const char* payload,
    const size_t payload_len,
    size_t& offset,
    std::string& outValue,
    std::string& errMsg)
{
    if (payload == nullptr)
    {
        errMsg = "payload is null";
        return false;
    }

    if (offset > payload_len || payload_len - offset < sizeof(uint16_t))
    {
        errMsg = "field length header overflow";
        return false;
    }

    uint16_t valueLength = 0;

    std::memcpy(&valueLength, payload + offset, sizeof(valueLength));

    offset += sizeof(valueLength);

    if (payload_len - offset < valueLength)
    {
        errMsg = "payload length overflow";
        return false;
    }

    outValue.assign(payload + offset,valueLength);

    offset += valueLength;

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
