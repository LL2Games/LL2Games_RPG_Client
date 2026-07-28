#include "PortalPacketHandler.h"
#include "stbNetworkDebug.h"
#include "PacketParser.h"
#include "stbSceneManager.h"
#include "PlayerManager.h"
#include "stbPlayer.h"
#include "stbTransform.h"
#include "MapScene.h"
#include "Portal.h"

std::string PortalPacketHandler::s_pendingPortalId{};

#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()

void PortalPacketHandler::HandleMoveMap(const ParsedPacket& pkt)
{
	try
	{
		size_t offset = 0;
		const char* data = pkt.payload.c_str();
		size_t payloadSize = pkt.payload.size();

		std::string status;
		std::string errMsg;

		int destinationMapId = 0;
		float spawnX = 0.0f;
		float spawnY = 0.0f;
		/*
			payload.push_back(std::to_string(destinationMapId));
			payload.push_back(std::to_string(spawnPosition.xPos));
			payload.push_back(std::to_string(spawnPosition.yPos));
		*/
		if (!PacketParser::ParseLengthPrefixedString(
			data,
			payloadSize,
			offset,
			status,
			errMsg))
		{
			ResetPendingPortal();
			throw std::runtime_error(errMsg);
		}

		ResetPendingPortal();

		if (status != "ok")
		{
			std::string serverError;

			PacketParser::ParseLengthPrefixedString(
				data,
				payloadSize,
				offset,
				serverError,
				errMsg);

		
			OutputDebugStringA(("포탈 이동 실패: " + serverError + "\n").c_str());

			return;
		}
		if(!PacketParser::ParseNextIntField(pkt.payload.c_str(), payloadSize, offset, destinationMapId, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if(!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, spawnX, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		if (!PacketParser::ParseNextFloatField(pkt.payload.c_str(), payloadSize, offset, spawnY, errMsg))
		{
			throw std::runtime_error(errMsg);
		}

		const std::wstring sceneName = L"Map_" + std::to_wstring(destinationMapId);

		stb::Scene* scene = M_SCENEMANAGER->LoadScene(sceneName);

		if (scene == nullptr)
		{
			throw std::runtime_error("destination scene not found");
		}

		stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();

		if (player == nullptr)
		{
			throw std::runtime_error("local player is nullptr");
		}

		stb::Transform* transform = player->GetComponent<stb::Transform>();

		if (transform == nullptr)
		{
			throw std::runtime_error("player transform is nullptr");
		}

		stb::math::Vector2 spawnPostion(spawnX, spawnY);

		transform->SetPosition(spawnPostion);

		player->GetPlayerLocation()->pos = spawnPostion;

		player->GetPlayerLocation()->mapId = destinationMapId;

		OutputDebugStringA("포탈 맵 이동 완료\n");
	}
	catch(const std::exception& e)
	{
		OutputDebugStringA(e.what());
		OutputDebugStringA("\n");
	}
	catch(...)
	{
		OutputDebugStringA("예상치 못한 에러 발생\n");
	}

}
void PortalPacketHandler::SendPortalEnter(std::string portalId)
{
	s_pendingPortalId = portalId;

	std::vector<std::string> data = { portalId };

	stb::NetworkManager::getInstance()->SendPacket(PKT_PORTAL_ENTER, data);	

	OutputDebugStringA("SendPortalEnter Success \n");
}


void PortalPacketHandler::ResetPendingPortal()
{
	stb::Scene* activeScene = M_SCENEMANAGER->GetActiveScene();

	MapScene* currentMap = dynamic_cast<MapScene*>(activeScene);

	if (currentMap == nullptr)
		return;

	Portal* portal = currentMap->FindPortal(s_pendingPortalId);

	if (portal != nullptr)
		portal->ResetTransition();
}
