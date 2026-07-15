#pragma once
#include "CommonInclude.h"

namespace stb
{
	class Player;
}

enum class AttackDirection : int
{
	Left = -1,
	Right = 1
};

class CombatSystem
{
public:
	CombatSystem();
	~CombatSystem() {};

	void SetOwner(stb::Player* player) { m_player = player; }

	bool TryAttack(int skillId);
	bool TryBasicAttack();
	bool CanAttack(int skillId);
	bool CanBasicAttack();

private:
	stb::Player* m_player;

	std::string m_debugMsg;


};

