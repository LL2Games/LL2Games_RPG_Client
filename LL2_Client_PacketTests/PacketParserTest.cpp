#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketParser.h"

namespace
{
    bool Check(const bool condition,const char* message)
    {
        if (!condition)
        {
            std::cerr << "    " << message << '\n';
            return false;
        }

        return true;
    }

    bool TestMaximumPacketAccepted()
    {
        const std::size_t maximumBodySize =PacketLimits::kMaxPacketSize - sizeof(PacketHeader);

        const std::string body(maximumBodySize, 'A');
        const std::string packet = PacketParser::MakePacket(PKT_CHAT, body);

        if (!Check(packet.size() == PacketLimits::kMaxPacketSize,"maximum packet size mismatch"))
        {
            return false;
        }

        std::vector<char> buffer(packet.begin(),packet.end());

        const ParseResult result =PacketParser::TryParse(buffer);

        return
            Check(result.status == ParseStatus::Complete,
                "maximum packet was not accepted") &&
            Check(
                result.packet.type == PKT_CHAT,
                "packet type mismatch") &&
            Check(
                result.packet.payload == body,
                "packet payload mismatch") &&
            Check(
                buffer.empty(),
                "parsed packet remained in buffer");
    }

    bool TestOversizedPacketRejected()
    {
        const std::size_t oversizedBodySize = PacketLimits::kMaxPacketSize -
            sizeof(PacketHeader) + 1;

        bool exceptionThrown = false;

        try
        {
            PacketParser::MakePacket(
                PKT_CHAT,
                std::string(oversizedBodySize, 'A')
            );
        }
        catch (const std::length_error&)
        {
            exceptionThrown = true;
        }

        if (!Check(
            exceptionThrown,
            "oversized outgoing packet was accepted"))
        {
            return false;
        }

        PacketHeader header{};
        header.length = static_cast<uint16_t>(
            PacketLimits::kMaxPacketSize + 1
            );
        header.type = PKT_CHAT;

        std::vector<char> buffer(sizeof(header));
        std::memcpy(
            buffer.data(),
            &header,
            sizeof(header)
        );

        const ParseResult result =
            PacketParser::TryParse(buffer);

        return Check(
            result.status == ParseStatus::InvalidPacket,
            "oversized incoming packet was accepted"
        );
    }

    bool TestUndersizedPacketRejected()
    {
        PacketHeader header{};
        header.length = static_cast<uint16_t>(
            sizeof(PacketHeader) - 1
            );
        header.type = PKT_CHAT;

        std::vector<char> buffer(sizeof(header));
        std::memcpy(
            buffer.data(),
            &header,
            sizeof(header)
        );

        const ParseResult result =
            PacketParser::TryParse(buffer);

        return Check(
            result.status == ParseStatus::InvalidPacket,
            "packet smaller than header was accepted"
        );
    }

    bool TestPartialPacketNeedsMoreData()
    {
        const std::string body =
            PacketParser::MakeBody({ "partial-packet" });

        const std::string packet =
            PacketParser::MakePacket(PKT_CHAT, body);

        std::vector<char> buffer(
            packet.begin(),
            packet.begin() + 2
        );

        ParseResult result =
            PacketParser::TryParse(buffer);

        if (!Check(
            result.status == ParseStatus::NeedMoreData,
            "partial header was not held"))
        {
            return false;
        }

        buffer.insert(
            buffer.end(),
            packet.begin() + 2,
            packet.end() - 1
        );

        result = PacketParser::TryParse(buffer);

        if (!Check(
            result.status == ParseStatus::NeedMoreData,
            "partial body was not held"))
        {
            return false;
        }

        buffer.push_back(packet.back());

        result = PacketParser::TryParse(buffer);

        return
            Check(
                result.status == ParseStatus::Complete,
                "completed packet was not parsed") &&
            Check(
                result.packet.payload == body,
                "completed packet payload mismatch") &&
            Check(
                buffer.empty(),
                "completed packet remained in buffer");
    }

    bool TestCoalescedPacketsParsedInOrder()
    {
        const std::string firstBody =
            PacketParser::MakeBody({ "first" });

        const std::string secondBody =
            PacketParser::MakeBody({ "second" });

        const std::string firstPacket =
            PacketParser::MakePacket(
                PKT_LOGIN,
                firstBody
            );

        const std::string secondPacket =
            PacketParser::MakePacket(
                PKT_CHAT,
                secondBody
            );

        std::vector<char> buffer;
        buffer.insert(
            buffer.end(),
            firstPacket.begin(),
            firstPacket.end()
        );
        buffer.insert(
            buffer.end(),
            secondPacket.begin(),
            secondPacket.end()
        );

        const ParseResult firstResult =
            PacketParser::TryParse(buffer);

        if (!Check(
            firstResult.status == ParseStatus::Complete,
            "first packet was not parsed"))
        {
            return false;
        }

        if (!Check(
            firstResult.packet.type == PKT_LOGIN &&
            firstResult.packet.payload == firstBody,
            "first packet data mismatch"))
        {
            return false;
        }

        const ParseResult secondResult =
            PacketParser::TryParse(buffer);

        return
            Check(
                secondResult.status == ParseStatus::Complete,
                "second packet was not parsed") &&
            Check(
                secondResult.packet.type == PKT_CHAT &&
                secondResult.packet.payload == secondBody,
                "second packet data mismatch") &&
            Check(
                buffer.empty(),
                "coalesced packets remained in buffer");
    }

    bool TestTwoByteLengthPrefix()
    {
        const std::string expected(300, 'X');
        const std::string body =
            PacketParser::MakeBody({ expected });

        std::size_t offset = 0;
        std::string actual;
        std::string errorMessage;

        const bool parsed =
            PacketParser::ParseLengthPrefixedString(
                body.data(),
                body.size(),
                offset,
                actual,
                errorMessage
            );

        return
            Check(
                parsed,
                "two-byte length field was not parsed") &&
            Check(
                actual == expected,
                "field longer than 255 bytes was truncated") &&
            Check(
                offset == body.size(),
                "field parsing offset mismatch");
    }
}

int main()
{
    struct TestCase
    {
        const char* name;
        bool (*function)();
    };

    const TestCase tests[] =
    {
        {
            "maximum packet accepted",
            TestMaximumPacketAccepted
        },
        {
            "oversized packet rejected",
            TestOversizedPacketRejected
        },
        {
            "undersized packet rejected",
            TestUndersizedPacketRejected
        },
        {
            "partial packet retained",
            TestPartialPacketNeedsMoreData
        },
        {
            "coalesced packets parsed in order",
            TestCoalescedPacketsParsedInOrder
        },
        {
            "two-byte length prefix parsed",
            TestTwoByteLengthPrefix
        },
    };

    int failureCount = 0;

    for (const TestCase& test : tests)
    {
        try
        {
            if (test.function())
            {
                std::cout
                    << "[PASS] "
                    << test.name
                    << '\n';
            }
            else
            {
                std::cerr
                    << "[FAIL] "
                    << test.name
                    << '\n';

                ++failureCount;
            }
        }
        catch (const std::exception& exception)
        {
            std::cerr
                << "[FAIL] "
                << test.name
                << ": "
                << exception.what()
                << '\n';

            ++failureCount;
        }
    }

    if (failureCount != 0)
    {
        std::cerr
            << failureCount
            << " test(s) failed\n";

        return 1;
    }

    std::cout << "All packet parser tests passed\n";
    return 0;
}
