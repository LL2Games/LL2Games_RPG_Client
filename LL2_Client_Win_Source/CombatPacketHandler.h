#pragma once
#include "Packet.h"
#include "CommonInclude.h"
#include "Inventory_Info.h"


class CombatPacketHandler
{
public:
	static void Execute(const ParsedPacket& pkt);
	static void HandlerMosterDamage(const ParsedPacket& pkt);
	static void HandleAttackResult(const ParsedPacket& pkt);
	static void HandleOtherPlayerAttack(const ParsedPacket& pkt);
	static void SendBasicAttack(int dir);
	static void SendUseSkill(int SkillId, int dir);

private:
};


