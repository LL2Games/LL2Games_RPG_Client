#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "AnimationInfo.h"
#include "EquipeTypes.h"
#include "..\\LL2_Client_Win_lib\\\stbPlayer.h"
#include "stbOtherPlayer.h"
#include "playerInfo.h"
#include <nlohmann/json.hpp>

class PlayerAnimationManager : public stb::SingletonBase<PlayerAnimationManager>
{
public:
    bool Init();
    bool PreLoadAll();
    bool LoadJsonFile(const std::string& path, PlayerAnimationSet& animData);

    bool SetupPlayerAnimations(stb::Player* player , JobType jobtype, WeaponType weaponType);
    bool SetupOtherPlayerAnimations(stb::OtherPlayer* otherPlayer, JobType jobtype, WeaponType weaponType);
    void BindPlayerAnimationEvents(stb::Player* player, stb::Animator* animator);
    void BindOtherPlayerAnimationEvents(stb::OtherPlayer* otherPlayer, stb::Animator* animator);
public:
    const AnimationInfo* FindAnimationData(std::string AnimName) const;
    const PlayerAnimationSet* FindAnimationSet(JobType jobtype, WeaponType weaponType) const;
private:
    std::unordered_map<PlayerAnimationKey, PlayerAnimationSet, PlayerAnimationKeyHash> m_animationSets;
    std::string m_debugMsg;
};

