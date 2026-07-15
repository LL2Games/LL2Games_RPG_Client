#pragma once
#include "CommonInclude.h"

enum class WeaponType
{
	None = 290000,
	One_Hand = 290001,
	Two_Hand = 290002,
	Bow = 290003,
    Dagger = 290004,
    Staff = 290005,
};

namespace Weapon
{
	inline WeaponType StringToWeaponType(const std::string str)
	{
        if (str == "onehand_sword")
            return WeaponType::One_Hand;

        if (str == "twohand_sword")
            return WeaponType::Two_Hand;

        if (str == "Bow")
            return WeaponType::Bow;

        if (str == "Dagger")
            return WeaponType::Dagger;

        if (str == "Staff")
            return WeaponType::Staff;

        return WeaponType::None;
	}

    inline WeaponType IntToWeaponType(const int type)
    {
        switch (type)
        {
            case 1: return WeaponType::One_Hand;
            case 2: return WeaponType::Two_Hand;
            case 3: return WeaponType::Bow;
            case 4: return WeaponType::Dagger;
            case 5: return WeaponType::Staff;
            default : return WeaponType::None;
        }
    }

}