#include "MovementProtocol.h"
#include "PacketParser.h"
#include <charconv>
#include <cmath>
#include <stdexcept>
#include <string_view>

namespace
{
    class Reader
    {
    public:
        explicit Reader(const std::string& value) : payload(value) {}
        std::string Field()
        {
            std::string field, error;
            if (!PacketParser::ParseLengthPrefixedString(payload.data(), payload.size(), offset, field, error))
                throw std::runtime_error(error);
            return field;
        }
        template<class T> T Number()
        {
            const std::string field = Field();
            T value{};
            const auto result = std::from_chars(field.data(), field.data() + field.size(), value);
            if (result.ec != std::errc{} || result.ptr != field.data() + field.size())
                throw std::runtime_error("invalid or out-of-range movement number");
            return value;
        }
        float Float()
        {
            float value = Number<float>();
            if (!std::isfinite(value)) throw std::runtime_error("non-finite movement position/velocity");
            return value;
        }
        bool Finished() const { return offset == payload.size(); }
    private:
        const std::string& payload;
        std::size_t offset = 0;
    };
}

bool movement::ParseSnapshot(const std::string& payload, Snapshot& result, std::string& error)
{
    try
    {
        Reader r(payload);
        Snapshot s;
        s.mapId = r.Number<int>();
        const int kind = r.Number<int>();
        s.entityId = r.Number<int>();
        s.epoch = r.Number<int>();
        s.tick = r.Number<std::uint64_t>();
        s.sequence = r.Number<int>();
        s.position = {r.Float(), r.Float()};
        s.velocity = {r.Float(), r.Float()};
        const int mode = r.Number<int>();
        s.facing = r.Number<int>();
        s.climbableId = r.Number<int>();
        s.lifeState = r.Number<int>();
        s.hp = r.Number<int>();
        s.maxHp = r.Number<int>();
        if (!r.Finished() || s.mapId <= 0 || kind < 0 || kind > 1 || s.entityId <= 0 ||
            s.epoch < 0 || s.sequence < 0 || mode < 0 || mode > 3 ||
            (s.facing != -1 && s.facing != 1) || s.climbableId < 0 ||
            s.lifeState < 0 || s.lifeState > (kind == 0 ? 5 : 8) ||
            s.hp < 0 || s.maxHp < 0 || s.hp > s.maxHp ||
            (kind == 1 && s.sequence != 0) || (mode == 3 && s.climbableId == 0) ||
            (mode != 3 && s.climbableId != 0))
            throw std::runtime_error("invalid movement snapshot fields");
        s.kind = static_cast<Kind>(kind);
        s.mode = static_cast<Mode>(mode);
        result = s; // All-or-nothing application.
        error.clear();
        return true;
    }
    catch (const std::exception& e)
    {
        error = e.what();
        return false;
    }
}

bool movement::BuildInputFields(int mapId, int epoch, int sequence, const Input& input,
                                std::vector<std::string>& fields)
{
    if (mapId <= 0 || epoch < 0 || sequence <= 0 || input.horizontal < -1 ||
        input.horizontal > 1 || input.vertical < -1 || input.vertical > 1) return false;
    fields = {std::to_string(mapId), std::to_string(epoch), std::to_string(sequence),
              std::to_string(input.horizontal), std::to_string(input.vertical), input.jump ? "1" : "0"};
    return true;
}
