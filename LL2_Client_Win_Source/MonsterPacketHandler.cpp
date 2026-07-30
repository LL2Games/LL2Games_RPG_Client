#include "MonsterPacketHandler.h"
#include "PacketParser.h"
#include "MonsterInfo.h"
#include "MonsterManager.h"
#include "ProjectileManager.h"
#include "stbLogger.h"

#define M_MONSTERMANAGER stb::SingletonBase<MonsterManager>::getInstance()
#define M_PROJECTILEMANAGER stb::SingletonBase<ProjectileManager>::getInstance()


void MonsterPacketHandler::HandleS2C_SpawnMonster(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		int monsterSize = 0;

		// Packet 사이즈를 받아온다
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSize, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		for (size_t i = 0; i < monsterSize; i++)
		{
			MonsterSpawnInfo monsterSpawnInfo{};
			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSpawnInfo.monsterId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSpawnInfo.instanceId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, monsterSpawnInfo.pos.x, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, monsterSpawnInfo.pos.y, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSpawnInfo.dir, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSpawnInfo.moveSpeed, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSpawnInfo.curHp, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSpawnInfo.maxHp, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			int state = 0;

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, state, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			monsterSpawnInfo.state = monster::SetState(state);

			M_MONSTERMANAGER->SpawnMonster(monsterSpawnInfo);
		}


	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandleS2C_SpawnMonster] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러가 발생했습니다.");
		OutputDebugStringA("\n");
	}
}

void MonsterPacketHandler::HandleS2C_MonsterMove(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		int monsterSize = 0;

		// Packet 사이즈를 받아온다
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSize, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		for (size_t i = 0; i < monsterSize; i++)
		{
			MonsterUpdateInfo monsterUpdateInfo{};
			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.instanceId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			int state = 0;
			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, state, errMsg))
			{
				throw std::runtime_error(errMsg);
			}
			monsterUpdateInfo.state = monster::SetState(state);

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.dir, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, monsterUpdateInfo.pos.x, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, monsterUpdateInfo.pos.y, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.curHp, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.maxHp, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			M_MONSTERMANAGER->ApplyServerUpdate(monsterUpdateInfo);

		}


	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandleS2C_MonsterMove] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러가 발생했습니다.");
		OutputDebugStringA("\n");
	}

}

void MonsterPacketHandler::HandleS2C_RespawnMonster(const ParsedPacket& pkt)
{
	/*
	payload.push_back(std::to_string(monsters.size()));

	for(const auto& monster : monsters)
    {
        payload.push_back(std::to_string(monster->GetInstanceId()));
        payload.push_back(std::to_string(monster->GetId()));
        payload.push_back(std::to_string(monster->GetPos().xPos));
        payload.push_back(std::to_string(monster->GetPos().yPos));
        payload.push_back(std::to_string(monster->GetDir().xPos));
        payload.push_back(std::to_string(monster->GetCurrentHP()));
        payload.push_back(std::to_string(monster->GetMaxHP()));
        payload.push_back(std::to_string(monster->GetState()));
    }
	
	*/
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		int monsterSize = 0;

		// Packet 사이즈를 받아온다
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterSize, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		for (size_t i = 0; i < monsterSize; i++)
		{
			MonsterUpdateInfo monsterUpdateInfo{};

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.instanceId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.monsterId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, monsterUpdateInfo.pos.x, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, monsterUpdateInfo.pos.y, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.dir, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.curHp, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, monsterUpdateInfo.maxHp, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			int state = 0;

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, state, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			monsterUpdateInfo.state = monster::SetState(state);

			M_MONSTERMANAGER->RespawnMonster(monsterUpdateInfo);
		}


	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandleS2C_RespawnMonster] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러가 발생했습니다.");
		OutputDebugStringA("\n");
	}
}

void MonsterPacketHandler::HandleS2C_ProjectileMove(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();
		std::string errMsg;

		int projectileSize = 0;

		// Packet 사이즈를 받아온다
		if (!PacketParser::ParseNextIntField(data, payloadSize, offset, projectileSize, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		for (size_t i = 0; i < projectileSize; i++)
		{
			MonsterProjectileData projectileInfo{};
			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, projectileInfo.instanceId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, projectileInfo.projectileId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextIntField(data, payloadSize, offset, projectileInfo.ownerId, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, projectileInfo.dirX, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, projectileInfo.dirY, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, projectileInfo.range, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, projectileInfo.speed, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, projectileInfo.pos.x, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			if (!PacketParser::ParseNextFloatField(data, payloadSize, offset, projectileInfo.pos.y, errMsg))
			{
				throw std::runtime_error(errMsg);
			}

			M_PROJECTILEMANAGER->ApplyServerUpdate(projectileInfo);
		}


	}
	catch (const std::exception& e)
	{
		OutputDebugStringA("[HandleS2C_ProjectileMove] ");
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch (...)
	{
		OutputDebugStringA("예상치 못한 에러가 발생했습니다.");
		OutputDebugStringA("\n");
	}

}
