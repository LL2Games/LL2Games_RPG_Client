#include "stbPlayerScript.h"
#include "stbInput.h"
#include "stbTransform.h"
#include "stbTime.h"
#include "stbGameObject.h"
#include "stbNetworkDebug.h"
#include "..\\LL2_Client_Win_lib\\stbPlayer.h"
#include "PlayerManager.h"
#include "QuickSlotManager.h"
#include "UIManager.h"
#include "PlayerAnimationManager.h"
#include "TradePacketHandler.h"
#include "stbSceneManager.h"
#include "stbPlayScene.h"
#include "MovePacketHandler.h"


#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_TIME  stb::SingletonBase<stb::Time>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()
#define M_PLAYERANIMMANAGER stb::SingletonBase<PlayerAnimationManager>::getInstance()
#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()

namespace stb
{
	PlayerScript::PlayerScript()
		: mNetworkSendTimer(0.0f)
		, mHead(nullptr)
		, mSword(nullptr)
		, m_player(nullptr)
		, mAttackTimer(0.0f)
		, mAttackDuration(0.35f)
		, m_animator(nullptr)
		, m_quickSlotManager(nullptr)
	{

	}

	PlayerScript::~PlayerScript()
	{

	}

	void PlayerScript::Initialize()
	{	

	}	
		
	void PlayerScript::Update()
	{
		if (m_player == nullptr) return;

		if (M_UIMANAGER->IsInputFocused())
			return;

		HandleCombatInput();
		HandleInput();

		if (m_player->GetState() == PlayerState::Attack)
		{
			Idle(false);
			return;
		}

		Idle(true);
		
	}	
		
	void PlayerScript::LateUpdate()
	{

	}	
		 
	void PlayerScript::Render(HDC /*hdc*/)
	{

	}

	void PlayerScript::SetAnimator()
	{
		if (m_player == nullptr) return;

		m_animator = m_player->GetAnimator();
	}

	void PlayerScript::UpdateAttackState()
	{
		stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();

		if (player == nullptr)
			return;

		mAttackTimer += M_TIME->GetDeltaTime();

		if (mAttackTimer < mAttackDuration)
			return;

		mAttackTimer = 0.0f;

		if (IsMoveInputPressed())
		{
			player->SetState(PlayerState::Walk);
			OutputDebugStringA("Attack End -> Move\n");
		}
		else
		{
			player->SetState(PlayerState::Idle);
			OutputDebugStringA("Attack End -> Idle\n");
		}
	}	

	void PlayerScript::Idle(bool changeState)
	{
		if (M_UIMANAGER->IsInputFocused())
			return; 
		Transform* tr = GetOwner()->GetComponent<Transform>();
		if (tr == nullptr)
			return;

		Vector2 pos = tr->GetPosition();
		bool moved = false;

		if (m_player->GetState() != PlayerState::Attack)
		{
			if (M_INPUT->GetAction(eActionCode::MoveRight))
			{
				pos.x += 200.0f * M_TIME->GetDeltaTime();
				m_player->SetFacing(FacingDirection::Right);
				m_animator->SetFlipX(true);
				moved = true;
			}

			if (M_INPUT->GetAction(eActionCode::MoveLeft))
			{
				pos.x -= 200.0f * M_TIME->GetDeltaTime();
				m_player->SetFacing(FacingDirection::Left);
				m_animator->SetFlipX(false);
				moved = true;
			}

			if (M_INPUT->GetAction(eActionCode::MoveUp))
			{
				pos.y -= 200.0f * M_TIME->GetDeltaTime();
				moved = true;
			}

			if (M_INPUT->GetAction(eActionCode::MoveDown))
			{
				pos.y += 200.0f * M_TIME->GetDeltaTime();
				moved = true;
			}

			tr->SetPosition(pos);
			if (m_player->GetPlayerLocation() != nullptr)
			{
				m_player->GetPlayerLocation()->pos = pos;
			}
		}
		
		if (changeState)
		{
			stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();
			if (player != nullptr)
			{
				if (moved)
					player->SetState(PlayerState::Walk);
				else
					player->SetState(PlayerState::Idle);
			}
		}

		SyncFollowers(pos);

		if (moved)
		{
			mNetworkSendTimer += M_TIME->GetDeltaTime();

			if (mNetworkSendTimer >= NETWORK_SEND_INTERVAL)
			{
				auto netMgr = stb::NetworkManager::getInstance();
				if (netMgr != nullptr && netMgr->IsConnected())
				{
					MovePacketHandler::SendPlayerMove(m_player);
					//stb::SendPlayerMove(pos.x, pos.y, 100.0f);
				}

				mNetworkSendTimer = 0.0f;
			}
		}
		else
		{
			mNetworkSendTimer = 0.0f;
		}
	}

	void PlayerScript::Move()
	{

	}

	void PlayerScript::Attack()
	{
		stb::Player* player = m_player;

		if (player == nullptr)
			return;

		if (player->GetCombatSystem() == nullptr)
			return;

		if (player->GetState() == PlayerState::Attack)
			return;

		if (player->GetCombatSystem()->TryBasicAttack())
		{
			player->SetState(PlayerState::Attack);
			mAttackTimer = 0.0f;

			OutputDebugStringA("Player Attack Start\n");
		}

	}

	void PlayerScript::Jump() 
	{
		if (m_player == nullptr)
			return;

		if (m_player->GetCombatSystem() == nullptr)
			return;

		if (m_player->GetState() == PlayerState::Jump)
			return;

		m_player->SetState(PlayerState::Jump);
		
	}

	void PlayerScript::PickUp()
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		if (tr == nullptr)
			return;

		stb::Scene* scene = M_SCENEMANAGER->GetActiveScene();

		if (scene == nullptr)
			return;

		PlayScene* playScene = dynamic_cast<PlayScene*>(scene);
		if (playScene == nullptr)
			return;

		DropItemManager* dropManager = playScene->GetDropItemManager();
		if (dropManager == nullptr)
			return;

		dropManager->RequestPickup(tr->GetPosition());
	}

	void PlayerScript::HandleInput()
	{
		//채팅 입력중 -> 단축키 차단
		if (M_UIMANAGER->IsInputFocused())
			return;

		KeyBindInfo bindInfo;

		if (M_INPUT->GetPressedBind(bindInfo))
		{
			ExecuteBind(bindInfo);
		}
	}

	void PlayerScript::HandleCombatInput()
	{
		//채팅 입력중 -> 이동/공격 차단
		if (M_UIMANAGER->IsInputFocused())
			return;

		if (M_INPUT->GetActionDown(eActionCode::Attack))
		{
			m_debugMsg = "HandleComabatInput is Pressed\n";
			OutputDebugStringA(m_debugMsg.c_str());
			Attack();
		}
	}

	void PlayerScript::ExecuteBind(const KeyBindInfo& bindInfo)
	{
		switch (bindInfo.type)
		{
		case eBindType::Action:
			ExecuteAction((eActionCode)bindInfo.value);
			break;
		case eBindType::Skill:
			// TODO :
			// SkillManager::GetInstance()->UseSkill(bindInfo.value);
			OutputDebugStringA("Skill Execute\n");
			break;
		case eBindType::Item:
			// TODO :
			// ItemManager::GetInstance()->UseItem(bindInfo.value);
			OutputDebugStringA("Item Execute\n");
			break;
		case eBindType::UI:
			OutputDebugStringA("UI Execute\n");
			break;
		case eBindType::QuickSlot:
			OutputDebugStringA("QuickSlot Execute\n");
			m_player->GetQuickSlotManager()->UseSlot(bindInfo.value);
			break;
		default:
			break;
		}
	}

	void PlayerScript::ExecuteAction(eActionCode action)
	{
		switch (action)
		{
		case eActionCode::Interact:
			OutputDebugStringA("Action : Interact\n");
			// TODO : 
			break;
		case eActionCode::Jump:
			Jump();
			OutputDebugStringA("Action : Jump\n");
			// TODO : 
			break;
		case eActionCode::Inventory:
			OutputDebugStringA("Action : Inventory\n");
			UIManager::getInstance()->ToggleInventory();
			// TODO : 
			break;
		case eActionCode::SkillWindow:
			OutputDebugStringA("Action : SkillWindow\n");
			// TODO :
#if 1 /* test */
			UIManager::getInstance()->ToggleTradeUI();
#endif /* test */
			break;
		case eActionCode::CharacterInfo:
			OutputDebugStringA("Action : Trade\n");
			UIManager::getInstance()->ToggleStat();
			break;
		case eActionCode::Trade:
			OutputDebugStringA("Action : Trade\n");
			//UIManager::getInstance()->OpenTradeUI();
			//UIManager::getInstance()->ToggleTradeUI(); //test ���
			UIManager::getInstance()->OpenReqTradeUI(); //��ȯ��û
			break;

		case eActionCode::TradeCancel:
			OutputDebugStringA("Action : Trade Cancel\n");
			UIManager::getInstance()->CloseTradeUI(); //��ȯ���
			break;
		case eActionCode::PickUp:
			OutputDebugStringA("Action : PickUp\n");
			PickUp();
			break;

		default:
			break;
		}
	}

	

	void PlayerScript::SyncFollowers(Vector2 pos)
	{
		if (mHead)
		{
			Transform* tr = mHead->GetComponent<Transform>();
			if (tr) tr->SetPosition(pos);
		}
		if (mSword)
		{
			Transform* tr = mSword->GetComponent<Transform>();
			if (tr) tr->SetPosition(Vector2(pos.x - 15.0f, pos.y + 7.0f));
		}
	}

	bool PlayerScript::IsMoveInputPressed() const
	{
		return M_INPUT->GetAction(eActionCode::MoveRight) ||
			M_INPUT->GetAction(eActionCode::MoveLeft) ||
			M_INPUT->GetAction(eActionCode::MoveUp) ||
			M_INPUT->GetAction(eActionCode::MoveDown);
	}

}
