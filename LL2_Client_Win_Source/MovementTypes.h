#pragma once
#include <cstdint>

namespace movement
{
    enum class Kind : int { Player = 0, Monster = 1 };
    enum class Mode : int { Grounded = 0, Rising = 1, Falling = 2, Climbing = 3 };
    enum class PlayerLife : int { Idle = 0, Move = 1, Jump = 2, Attack = 3, Stunned = 4, Dead = 5 };
    enum class MonsterLife : int
    {
        Idle = 0, Patrol = 1, Chase = 2, Move = 3, Dying = 4,
        Hit = 5, RangeAttack = 6, Dead = 7, None = 8
    };

    struct Point { float x = 0; float y = 0; };
    struct Input { int horizontal = 0; int vertical = 0; bool jump = false; };
    struct Snapshot
    {
        int mapId = 0;
        Kind kind = Kind::Player;
        int entityId = 0;
        int epoch = 0;
        std::uint64_t tick = 0;
        int sequence = 0; // Last accepted input, NOT a physics acknowledgement.
        Point position;
        Point velocity;
        Mode mode = Mode::Grounded;
        int facing = -1;
        int climbableId = 0;
        int lifeState = 0;
        int hp = 0;
        int maxHp = 0;
    };
    inline bool IsDead(const Snapshot& s)
    {
        return s.kind == Kind::Player ? s.lifeState == static_cast<int>(PlayerLife::Dead)
            : s.lifeState == static_cast<int>(MonsterLife::Dying) ||
              s.lifeState == static_cast<int>(MonsterLife::Dead);
    }
    inline bool IsStunned(const Snapshot& s)
    {
        return s.kind == Kind::Player && s.lifeState == static_cast<int>(PlayerLife::Stunned);
    }
    inline bool IsNewer(const Snapshot& a, const Snapshot& b)
    {
        return a.epoch > b.epoch || (a.epoch == b.epoch && a.tick > b.tick);
    }
    inline constexpr float PlayerFootOffset = 10.0f;
}
