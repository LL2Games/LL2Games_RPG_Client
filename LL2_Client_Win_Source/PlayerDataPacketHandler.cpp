#include "PlayerDataPacketHandler.h"
#include "PacketData.h"
#include "PacketParser.h"
#include "PlayerManager.h"
#include "PlayerManager.h"
#include "ExpBarUI.h"
#include "Stat.h"
#include "UIManager.h"

#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()



void PlayerDataPacketHandler::HandleLocalPlayerInfo(const ParsedPacket& pkt)
{
	try
	{

		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		auto playerManager = PlayerManager::getInstance();
		if (!playerManager)
		{
			throw std::runtime_error("playerManager is nullptr");
		}
		auto localPlayer = playerManager->GetLocalPlayer();
		if (!localPlayer)
		{
			throw std::runtime_error("localPlayer is nullptr");
		}
		PlayerIdentity playerIdentity = {};
		PlayerProfile playerProfile = {};
		PlayerLocation playerLocation = {};
		
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, playerIdentity.charId, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, playerProfile.name, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, playerProfile.job, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, playerProfile.level, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, playerLocation.mapId, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, playerLocation.pos.x, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, playerLocation.pos.y, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		localPlayer->SetPlayerInfo(playerIdentity, playerProfile, playerLocation);

		OutputDebugStringA("SetPlayerInfo Success\n");
		
	}
	catch(const std::exception& e)
	{
		OutputDebugStringA("[HandleLocalPlayerInfo] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch(...)
	{
		OutputDebugStringA("예상치 못한 에러가 발생했습니다.");
	}
}

void PlayerDataPacketHandler::HandleLocalPlayerStat(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		auto playerManager = PlayerManager::getInstance();
		if (!playerManager)
		{
			throw std::runtime_error("playerManager is nullptr");
		}
		auto localPlayer = playerManager->GetLocalPlayer();
		if (!localPlayer)
		{
			throw std::runtime_error("localPlayer is nullptr");
		}
		BaseStat baseStat = {};
		DerivedStat derived = {};
		ExpStat expStat = {};
		int cur_hp = 0;
		int cur_mp = 0;
		int remain_ap = 0;
	
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, baseStat.str, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, baseStat.dex, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, baseStat.intel, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, baseStat.luck, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, derived.maxHp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, derived.maxMp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, cur_hp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, cur_mp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, remain_ap, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, expStat.level, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextInt64Field(data, payloadSize, offset, expStat.exp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextInt64Field(data, payloadSize, offset, expStat.need_exp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		localPlayer->SetStat(baseStat, derived, expStat, cur_hp, cur_mp, remain_ap);

		OutputDebugStringA("SetPlayerStat Success\n");
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandleLocalPlayerStat] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러 발생\n");
	}
}

void PlayerDataPacketHandler::HandleLocalPlayerGetExp(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		auto playerManager = PlayerManager::getInstance();
		if (playerManager == nullptr)
		{
			throw std::runtime_error("playerManager is nullptr");
		}
		auto localPlayer = playerManager->GetLocalPlayer();
		if (localPlayer == nullptr)
		{
			throw std::runtime_error("localPlayer is nullptr");
		}

		Stat* stat = localPlayer->GetStat();

		if (stat == nullptr)
		{
			throw std::runtime_error("stat is nullptr");
		}
		
		ExpUpdateResult expResult{};
		
		bool isLevelUp = false;

		if (!PacketParser::ParseNextInt64Field(data, payloadSize, offset, expResult.gainedExp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, expResult.newLevel, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextInt64Field(data, payloadSize, offset, expResult.curExp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextInt64Field(data, payloadSize, offset, expResult.needExp, errMsg))
		{
			throw std::runtime_error(errMsg);
		}
		
		int levelUpValue = 0;
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, levelUpValue, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		expResult.levelUp = (levelUpValue != 0);

		stat->HandleExpGain(expResult);
		std::string DebugMsg;
		DebugMsg = "Exp : " + std::to_string(expResult.curExp) + "\n"
			+ "needExp :" + std::to_string(expResult.needExp) + "\n";
		OutputDebugStringA(DebugMsg.c_str());
		OutputDebugStringA("HandleLocalPlayerGetExp Success\n");
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandleLocalPlayerGetExp] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러 발생\n");
	}
}

void PlayerDataPacketHandler::HandlePlayerOnDamaged(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		auto playerManager = PlayerManager::getInstance();
		if (!playerManager)
		{
			throw std::runtime_error("playerManager is nullptr");
		}
		auto localPlayer = playerManager->GetLocalPlayer();
		if (!localPlayer)
		{
			throw std::runtime_error("localPlayer is nullptr");
		}
		
		std::vector<std::string> inputs;

		while (1)
		{
			std::string input;
			if (!PacketParser::ParseLengthPrefixedString(data, payloadSize, offset, input, errMsg))
			{
				break;
			}
			inputs.push_back(input);
		}

		if (inputs.size() == 6) //자기 자신의 온데미지
		{
			int player_id = std::stoi(inputs[0]);
			int attacker_instance_id = std::stoi(inputs[1]);
			int damage = std::stoi(inputs[2]);
			int cur_hp = std::stoi(inputs[3]);
			int max_hp = std::stoi(inputs[4]);
			int player_state = std::stoi(inputs[5]);

			//localPlayer->GetStat()->SetCurHp(cur_hp);
			//TODO
			//Player OnDamage 데미지 받는 형태
			localPlayer->OnDamaged(damage, cur_hp);
		}
		else if (inputs.size() == 4) //타인의 온데미지
		{
			int target_player_id = std::stoi(inputs[0]);
			int attacker_instance_id = std::stoi(inputs[1]);
			int damage = std::stoi(inputs[2]);
			int player_state = std::stoi(inputs[3]);
			
			auto targetPlayer = playerManager->FindPlayer(target_player_id);
			if (!targetPlayer)
			{
				throw std::runtime_error("targetPlayer is nullptr");
			}

			//TODO
			//TargetPlayer OnDamage 데미지 받는 형태
			targetPlayer->ShowDamageText(damage);
		}
		else
			throw std::runtime_error("HandlePlayerOnDamaged input error");

		OutputDebugStringA("HandlePlayerOnDamaged Success\n");
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandlePlayerOnDamaged] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러 발생\n");
	}
}