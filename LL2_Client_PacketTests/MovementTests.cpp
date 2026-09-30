#include "MovementProtocol.h"
#include "MovementStream.h"
#include "MovementMap.h"
#include "PacketParser.h"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace
{
    void Require(bool value, const char* reason)
    {
        if (!value) throw std::runtime_error(reason);
    }
    std::vector<std::string> Fields()
    {
        return {"100000000", "0", "42", "3", "18446744073709551615", "7",
            "100", "690", "0", "0", "0", "-1", "0", "0", "100", "100"};
    }
    movement::Snapshot Snapshot()
    {
        movement::Snapshot result;
        std::string error;
        Require(movement::ParseSnapshot(PacketParser::MakeBody(Fields()), result, error), "valid server snapshot rejected");
        return result;
    }
    void Protocol()
    {
        auto parsed = Snapshot();
        Require(parsed.tick == (std::numeric_limits<std::uint64_t>::max)(), "uint64 tick truncated");
        Require(parsed.position.y == 690 && parsed.sequence == 7, "origin or sequence changed");
        std::vector<std::string> input;
        Require(movement::BuildInputFields(100000000, 3, 1, {1, 0, true}, input), "valid input rejected");
        Require(input == std::vector<std::string>{"100000000", "3", "1", "1", "0", "1"}, "input not exact 6-field contract");
        Require(!movement::BuildInputFields(1, 3, 0, {}, input), "zero sequence accepted");
        Require(!movement::BuildInputFields(1, 3, 1, {2, 0, false}, input), "invalid horizontal accepted");
        auto fields = Fields();
        fields[1] = "1"; fields[5] = "0"; fields[13] = "7"; fields[14] = "0";
        std::string error;
        Require(movement::ParseSnapshot(PacketParser::MakeBody(fields), parsed, error) && movement::IsDead(parsed), "monster DEAD=7 not recognized");
        fields[13] = "4";
        Require(movement::ParseSnapshot(PacketParser::MakeBody(fields), parsed, error) && movement::IsDead(parsed), "monster DYING=4 not recognized");
        fields = Fields(); fields[13] = "5"; fields[14] = "0";
        Require(movement::ParseSnapshot(PacketParser::MakeBody(fields), parsed, error) && movement::IsDead(parsed), "player DEAD=5 not recognized");
    }
    void InvalidPackets()
    {
        const std::pair<std::size_t, const char*> invalid[] = {
            {0, "-1"}, {1, "2"}, {2, "0"}, {3, "2147483648"}, {4, "-1"},
            {4, "18446744073709551616"}, {5, "-1"}, {6, "nan"}, {7, "inf"},
            {8, "1e999"}, {9, "12garbage"}, {10, "4"}, {11, "0"}, {12, "-1"},
            {13, "6"}, {14, "101"}, {15, "-1"}, {6, ""}, {3, "1.5"}
        };
        for (const auto& [index, value] : invalid)
        {
            auto fields = Fields(); fields[index] = value;
            auto out = Snapshot(); out.entityId = 999;
            std::string error;
            Require(!movement::ParseSnapshot(PacketParser::MakeBody(fields), out, error), "invalid field accepted");
            Require(out.entityId == 999, "invalid packet partially applied");
        }
        const auto good = PacketParser::MakeBody(Fields());
        for (std::size_t n = 0; n < good.size(); ++n)
        {
            movement::Snapshot out;
            std::string error;
            Require(!movement::ParseSnapshot(good.substr(0, n), out, error), "truncated payload accepted");
        }
        auto fields = Fields(); fields.push_back("unexpected");
        movement::Snapshot out; std::string error;
        Require(!movement::ParseSnapshot(PacketParser::MakeBody(fields), out, error), "17th field accepted");
        fields = Fields(); fields[10] = "3";
        Require(!movement::ParseSnapshot(PacketParser::MakeBody(fields), out, error), "climbing without ID accepted");
    }
    void EpochAndInterpolation()
    {
        movement::Stream stream;
        auto s = Snapshot(); s.tick = 100;
        Require(stream.Push(s), "first snapshot rejected");
        movement::Snapshot view;
        Require(stream.Update(0, view) && view.position.x == 100, "first snapshot not immediate");
        s.tick = 103; s.position.x = 200;
        Require(stream.Push(s), "new tick rejected");
        stream.Update(0.025f, view);
        Require(std::abs(view.position.x - 150) < 0.01f, "50ms interpolation midpoint wrong");
        stream.Update(10, view);
        Require(view.position.x == 200, "extrapolation beyond final snapshot");
        Require(!stream.Push(s), "duplicate tick accepted");
        s.tick = 102; Require(!stream.Push(s), "old tick accepted");
        stream.Suspend();
        s.tick = 104; Require(!stream.Push(s), "old epoch resumed same-map transition");
        s.epoch = 4; s.tick = 1; s.position.x = 300;
        Require(stream.Push(s), "new epoch with smaller tick rejected");
        stream.Update(0, view);
        Require(view.position.x == 300, "new epoch blended across teleport");
        s.epoch = 3; s.tick = 999; Require(!stream.Push(s), "old epoch with newer tick accepted");
        stream.Reset(); Require(stream.Push(s), "new connection retained old epoch");
        s.tick = 1000; s.mode = movement::Mode::Falling;
        stream.Push(s); stream.Update(0.049f, view);
        s.tick = 1001;
        stream.Push(s); stream.Update(0.049f, view);
        Require(view.mode == movement::Mode::Falling, "frequent snapshots prevented mode transition");
        s.tick = 1002; s.mode = movement::Mode::Grounded;
        stream.Push(s); stream.Update(0.049f, view);
        s.tick = 1003;
        stream.Push(s); stream.Update(0.049f, view);
        Require(view.mode == movement::Mode::Grounded, "frequent snapshots prevented landing");
        stream.Reset();
        s.position.x = (std::numeric_limits<float>::max)();
        stream.Push(s);
        ++s.tick; s.position.x = -(std::numeric_limits<float>::max)();
        stream.Push(s); stream.Update(0.025f, view);
        Require(std::isfinite(view.position.x), "finite endpoints overflowed interpolation");
    }
    void DeathAndRevival()
    {
        movement::Stream stream;
        auto s = Snapshot(); s.tick = 1;
        stream.Push(s); stream.Suspend(true);
        s.tick = 2;
        Require(!stream.Push(s), "live old packet undid death event");
        s.lifeState = 5; s.hp = 0;
        Require(stream.Push(s), "same-epoch authoritative death rejected");
        movement::Snapshot displayed;
        stream.Update(0, displayed);
        Require(movement::IsDead(displayed), "death not applied immediately");
        s.tick = 3; s.lifeState = 0; s.hp = 100;
        Require(!stream.Push(s), "same-epoch live update resurrected player");
        s.epoch += 1; s.tick = 0;
        Require(stream.Push(s), "live new epoch rejected");
        Require(stream.Ready() && !movement::IsDead(stream.Latest()), "revival did not resume");
        // A later legacy 'ok' intentionally does not call Suspend/Reset.
    }
    void InputTiming()
    {
        movement::InputSchedule schedule;
        int sequence = 0;
        Require(schedule.Poll({}, 0, false, sequence) && sequence == 1, "first sequence not 1");
        Require(schedule.Poll({1, 0, true}, 0, false, sequence) && sequence == 2, "jump/change not immediate");
        Require(!schedule.Poll({1, 0, false}, 0.01f, false, sequence), "jump repeated on next frame");
        Require(schedule.Poll({1, 0, false}, 0.10f, false, sequence), "heartbeat missing");
        Require(schedule.Poll({}, 0, true, sequence), "focus/UI zero input not immediate");
        Require(!schedule.Poll({}, 0.01f, false, sequence), "neutral spammed every frame");
        schedule.Reset();
        Require(schedule.Poll({}, 0, false, sequence) && sequence == 1, "epoch did not reset sequence");
        schedule.Reset((std::numeric_limits<int>::max)());
        Require(!schedule.Poll({}, 1, true, sequence), "sequence overflowed");
    }
    void MapData()
    {
        std::ifstream file("docs/movement_data/maps.json");
        Require(file.is_open(), "run tests from repository root to load server map fixture");
        nlohmann::json exported; file >> exported;
        for (const auto& map : exported.at("maps"))
        {
            const auto geometry = movement::ParseMapGeometry(map.at("physics"));
            Require(geometry.platforms.size() == 3 && geometry.climbables.size() == 2, "server geometry incomplete");
            Require(geometry.FindClimbable(1)->ladder && !geometry.FindClimbable(2)->ladder, "rope/ladder swapped");
            std::ifstream clientFile("LL2_Client_Win/Data/Maps/" + std::to_string(map.at("mapId").get<int>()) + ".json");
            nlohmann::json client; clientFile >> client;
            Require(client.at("physics") == map.at("physics"), "client/server physics export mismatch");
            for (const auto& portal : map.at("portals"))
            {
                bool found = false;
                for (const auto& c : client.at("portals"))
                    if (c.at("id") == portal.at("id"))
                    {
                        for (auto it = portal.begin(); it != portal.end(); ++it)
                            Require(c.at(it.key()) == it.value(), "server portal field mismatch");
                        Require(c.contains("texture") && c.contains("renderSize"), "client portal art removed");
                        found = true;
                    }
                Require(found, "server portal absent");
            }
        }
        auto bad = exported.at("maps")[0].at("physics");
        bad["climbables"][1]["topPlatformId"] = 999;
        bool rejected = false;
        try { (void)movement::ParseMapGeometry(bad); } catch (const std::exception&) { rejected = true; }
        Require(rejected, "rope without exit platform accepted");
    }
}

int RunMovementTests()
{
    const std::pair<const char*, void(*)()> tests[] = {
        {"movement wire contract", Protocol}, {"movement invalid/truncated payloads", InvalidPackets},
        {"movement epoch/interpolation", EpochAndInterpolation}, {"movement death/revival", DeathAndRevival},
        {"movement input edges/heartbeat", InputTiming}, {"movement server map export", MapData}
    };
    int failed = 0;
    for (const auto& [name, run] : tests)
    {
        try { run(); std::cout << "[PASS] " << name << '\n'; }
        catch (const std::exception& e) { ++failed; std::cerr << "[FAIL] " << name << ": " << e.what() << '\n'; }
    }
    return failed;
}
