#pragma once
#include "playerInfo.h"
struct AttackResult
{
    int playerId;
    int skillId;
    int monster_instance_id;
    int damage;
    int cur_hp;
    int max_hp;
    int isDead;
};

struct OtherPlayerAttack
{
    int playerId;
    int skillId;
    int attackDir;
    PlayerState state;
};