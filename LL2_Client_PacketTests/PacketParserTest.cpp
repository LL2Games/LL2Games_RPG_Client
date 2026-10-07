#include <WinSock2.h>
#include "Projectile.h"
#include "ProjectileManager.h"
#include "MonsterPacketHandler.h"
#include <cmath>
#include "Monster.h"
#include "../LL2_Client_Win/GameSystemKeyPolicy.h"

#pragma comment(lib, "Ws2_32.lib")

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "PacketParser.h"
int RunMovementTests();

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
            Check(result.status == ParseStatus::Complete, "maximum packet was not accepted") &&
            Check(result.packet.type == PKT_CHAT, "packet type mismatch") &&
            Check(result.packet.payload == body, "packet payload mismatch") &&
            Check(buffer.empty(),"parsed packet remained in buffer");
    }

    bool TestOversizedPacketRejected()
    {
        const std::size_t oversizedBodySize = PacketLimits::kMaxPacketSize - sizeof(PacketHeader) + 1;

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
        header.length = htons(static_cast<uint16_t>(PacketLimits::kMaxPacketSize + 1));
        header.type = htons(static_cast<uint16_t>(PKT_CHAT));

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
        header.length = htons(static_cast<uint16_t>(sizeof(PacketHeader) - 1));
        header.type = htons(static_cast<uint16_t>(PKT_CHAT));

        std::vector<char> buffer(sizeof(header));
        std::memcpy(buffer.data(),&header,sizeof(header));

        const ParseResult result =PacketParser::TryParse(buffer);

        return Check(result.status == ParseStatus::InvalidPacket,"packet smaller than header was accepted");
    }

    bool TestPartialPacketNeedsMoreData()
    {
        const std::string body = PacketParser::MakeBody({ "partial-packet" });
        const std::string packet = PacketParser::MakePacket(PKT_CHAT, body);

        std::vector<char> buffer(packet.begin(), packet.begin() + 2);

        ParseResult result = PacketParser::TryParse(buffer);

        if (!Check(result.status == ParseStatus::NeedMoreData, "partial header was not held"))
        {
            return false;
        }

        buffer.insert(buffer.end(), packet.begin() + 2, packet.end() - 1);
        result = PacketParser::TryParse(buffer);

        if (!Check(result.status == ParseStatus::NeedMoreData, "partial body was not held"))
        {
            return false;
        }

        buffer.push_back(packet.back());

        result = PacketParser::TryParse(buffer);

        return
            Check(result.status == ParseStatus::Complete, "completed packet was not parsed") &&
            Check(result.packet.payload == body, "completed packet payload mismatch") &&
            Check(buffer.empty(), "completed packet remained in buffer");
    }

    bool TestCoalescedPacketsParsedInOrder()
    {
        const std::string firstBody = PacketParser::MakeBody({ "first" });
        const std::string secondBody = PacketParser::MakeBody({ "second" });
        const std::string firstPacket =PacketParser::MakePacket(PKT_LOGIN, firstBody);
        const std::string secondPacket =PacketParser::MakePacket(PKT_CHAT, secondBody);

        std::vector<char> buffer;
        buffer.insert(buffer.end(), firstPacket.begin(), firstPacket.end());
        buffer.insert(buffer.end(),secondPacket.begin(),secondPacket.end());

        const ParseResult firstResult = PacketParser::TryParse(buffer);

        if (!Check(firstResult.status == ParseStatus::Complete, "first packet was not parsed"))
        {
            return false;
        }

        if (!Check(firstResult.packet.type == PKT_LOGIN && firstResult.packet.payload == firstBody, "first packet data mismatch"))
        {
            return false;
        }

        const ParseResult secondResult = PacketParser::TryParse(buffer);

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
        const std::string body = PacketParser::MakeBody({ expected });

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
            Check(parsed,"two-byte length field was not parsed") &&
            Check(actual == expected,"field longer than 255 bytes was truncated") &&
            Check(offset == body.size(), "field parsing offset mismatch");
    }

    bool TestNetworkByteOrder()
    {
        const std::string packet = PacketParser::MakePacket(0x1234, "");

        if (!Check( packet.size() == sizeof(PacketHeader), "empty packet size mismatch"))
        {
            return false;
        }

        if (!Check(
            static_cast<unsigned char>(packet[0]) == 0x00 &&
            static_cast<unsigned char>(packet[1]) == 0x04 &&
            static_cast<unsigned char>(packet[2]) == 0x12 &&
            static_cast<unsigned char>(packet[3]) == 0x34,
            "packet header is not network byte order"))
        {
            return false;
        }

        const std::string body = PacketParser::MakeBody({std::string(300, 'X')});

        if (!Check(body.size() >= sizeof(uint16_t), "encoded body is too small"))
        {
            return false;
        }

        if (!Check(
            static_cast<unsigned char>(body[0]) == 0x01 &&
            static_cast<unsigned char>(body[1]) == 0x2C,
            "field length is not network byte order"))
        {
            return false;
        }

        std::vector<char> receivedPacket{
            static_cast<char>(0x00),
            static_cast<char>(0x04),
            static_cast<char>(0x12),
            static_cast<char>(0x34)
        };

        const ParseResult result = PacketParser::TryParse(receivedPacket);

        return
            Check(result.status == ParseStatus::Complete, "network byte order packet was not parsed") &&
            Check(result.packet.type == 0x1234, "network byte order packet type mismatch") &&
            Check(receivedPacket.empty(), "parsed packet remained in buffer");
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
            "network byte order",
            TestNetworkByteOrder
        },
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

    int failureCount = RunMovementTests();
    try
    {
        for (UINT message : {WM_SYSKEYDOWN, WM_SYSKEYUP})
        {
            for (WPARAM key : {WPARAM(VK_MENU), WPARAM(VK_UP), WPARAM(VK_DOWN), WPARAM(VK_LEFT), WPARAM(VK_RIGHT), WPARAM(VK_SPACE)})
                if (!ConsumeGameplaySystemKey(message, key))
                    throw std::runtime_error("Alt gameplay combination escaped to system menus");
            for (WPARAM key : {WPARAM(VK_TAB), WPARAM(VK_ESCAPE), WPARAM(VK_F4)})
                if (ConsumeGameplaySystemKey(message, key))
                    throw std::runtime_error("OS switch/close shortcut was blocked");
        }
        if (ConsumeGameplaySystemKey(WM_KEYDOWN, VK_UP) ||
            !ConsumeGameplaySystemKey(WM_SYSCHAR, 'x') ||
            !ConsumeGameplaySystemKey(WM_SYSDEADCHAR, '^'))
            throw std::runtime_error("normal/system character policy mismatch");
        std::cout << "[PASS] Alt ladder keys and OS shortcut routing\n";
    }
    catch (const std::exception& exception)
    {
        ++failureCount;
        std::cerr << "[FAIL] Alt input: " << exception.what() << '\n';
    }
    try
    {
        auto require = [](bool ok, const char* message) { if (!ok) throw std::runtime_error(message); };
        // Exercise the actual 9-field spawn receiver with decimal directions/coordinates.
        auto* manager = ProjectileManager::getInstance();
        manager->Clear("test_start");
        ParsedPacket packet{};
        packet.payload = PacketParser::MakeBody({"1", "42", "7", "0", "-0.997971", "0.063671",
            "1000.5", "100.25", "200.5", "50.25"});
        MonsterPacketHandler::HandleS2C_ProjectileMove(packet);
        auto* projectile = manager->FindProjectile(42);
        require(projectile != nullptr, "decimal spawn packet was rejected");
        auto* transform = projectile->GetComponent<stb::Transform>();
        require(transform->GetPosition().x == 200.5f && transform->GetPosition().y == 50.25f,
            "spawn coordinate changed axis or scale");
        manager->Update(0.5f);
        const auto moved = transform->GetPosition();
        require(std::abs(moved.x - (200.5f - 0.997971f * 100.25f * 0.5f)) < 0.001f &&
            std::abs(moved.y - (50.25f + 0.063671f * 100.25f * 0.5f)) < 0.001f,
            "decimal diagonal direction was truncated or rescaled");
        MonsterPacketHandler::HandleS2C_ProjectileMove(packet);
        require(manager->FindProjectile(42) == projectile && transform->GetPosition().x == moved.x,
            "duplicate spawn reset the projectile");
        manager->Update(2.5f);
        require(manager->FindProjectile(42) != nullptr, "spawn-only projectile expired on receive timeout");
        manager->Update(7.0f);
        require(manager->FindProjectile(42) == nullptr, "projectile survived actual travel range");
        MonsterPacketHandler::HandleS2C_ProjectileMove(packet);
        require(manager->FindProjectile(42) == nullptr, "duplicate packet revived retired ID");
        manager->Clear("map_exit");
        MonsterPacketHandler::HandleS2C_ProjectileMove(packet);
        require(manager->FindProjectile(42) != nullptr, "map exit did not allow ID reuse");
        manager->Clear("test_end");

        // Two clients given the same spawn and elapsed time follow the same world trajectory.
        MonsterProjectileData info{};
        info.instanceId = 99; info.dirX = -0.997971f; info.dirY = 0.063671f;
        info.pos = {200.5f, 50.25f}; info.speed = 100.25f; info.range = 1000.5f;
        Projectile first, second;
        first.Initialize(); second.Initialize();
        first.InitFromServer(info); second.InitFromServer(info);
        first.Update(0.25f); first.Update(0.25f); second.Update(0.5f);
        const auto firstPosition = first.GetComponent<stb::Transform>()->GetPosition();
        const auto secondPosition = second.GetComponent<stb::Transform>()->GetPosition();
        require(std::abs(firstPosition.x - secondPosition.x) < 0.001f &&
            std::abs(firstPosition.y - secondPosition.y) < 0.001f &&
            std::abs(first.GetTravelledDistance() - second.GetTravelledDistance()) < 0.001f,
            "same spawn diverged between client simulations");

        Monster monster;
        monster.Initialize();
        MonsterSpawnInfo spawn{};
        spawn.state = MonsterState::E_Idle;
        monster.InitFromSpawn(spawn);
        MonsterUpdateInfo update{};
        update.pos = {200, 50}; update.dir = 1; update.state = MonsterState::E_Move;
        monster.ApplyServerUpdate(update);
        monster.Update(0.05f);
        require(monster.GetComponent<stb::Transform>()->GetPosition().x == 200,
            "legacy monster movement remained at spawn");
        auto* animator = monster.GetComponent<stb::Animator>();
        animator->CreateAnimation(L"attack", nullptr, {}, {1, 1}, {}, 1, 1.0f);
        animator->CreateAnimation(L"idle", nullptr, {}, {1, 1}, {}, 1, 1.0f);
        movement::Snapshot snapshot;
        snapshot.kind = movement::Kind::Monster;
        snapshot.epoch = 1; snapshot.tick = 1;
        snapshot.hp = snapshot.maxHp = 100;
        snapshot.position = {300, 50};
        snapshot.lifeState = static_cast<int>(movement::MonsterLife::RangeAttack);
        monster.ApplyMovementSnapshot(snapshot);
        monster.Update(0.05f);
        require(animator->IsPlaying(L"attack"), "movement visual overwrote ranged attack");
        update.pos = {900, 50};
        monster.ApplyServerUpdate(update);
        monster.Update(0.05f);
        require(monster.GetComponent<stb::Transform>()->GetPosition().x == 300,
            "legacy packet overwrote authoritative snapshot");
        std::cout << "[PASS] projectile decimal spawn, range, duplicate/map lifecycle, client trajectories and monster visuals\n";
    }
    catch (const std::exception& exception)
    {
        ++failureCount;
        std::cerr << "[FAIL] combat visuals: " << exception.what() << '\n';
    }

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
