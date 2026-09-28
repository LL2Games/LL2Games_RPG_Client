#include "..\\LL2_Client_Win_lib\\stbPlayer.h"

#include "stbPlayerScript.h"
#include "stbInput.h"
#include "stbTransform.h"
#include "stbTime.h"
#include "stbGameObject.h"
#include "stbNetworkDebug.h"
#include "PlayerManager.h"
#include "QuickSlotManager.h"
#include "UIManager.h"
#include "PlayerAnimationManager.h"
#include "TradePacketHandler.h"
#include "stbSceneManager.h"
#include "stbPlayScene.h"
#include "MovePacketHandler.h"
#include "SkillDataManager.h"
#include "SkillEffectManager.h"

#include <cmath>


#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_TIME  stb::SingletonBase<stb::Time>::getInstance()
#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
#define M_UIMANAGER stb::SingletonBase<UIManager>::getInstance()
#define M_PLAYERANIMMANAGER stb::SingletonBase<PlayerAnimationManager>::getInstance()
#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()
#define M_SKILLDATAMANAGER stb::SingletonBase<SkillDataManager>::getInstance()
#define M_SKILLEFFECTMANAGER stb::SingletonBase<SkillEffectManager>::getInstance()

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

		if (M_UIMANAGER->IsGameplayInputBlocked())
			return;

		HandleCombatInput();
		HandleInput();

		if (m_player->GetState() >= PlayerState::Attack && m_player->GetState() < PlayerState::Skill_End)
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
		if (M_UIMANAGER->IsGameplayInputBlocked())
		{
			return;
		}

		Transform* tr = GetOwner()->GetComponent<Transform>();

		if (tr == nullptr)
		{
			return;
		}

		Vector2 pos = tr->GetPosition();
		bool moved = false;

		const float moveSpeed = m_player->GetPlayerMoveSpeed();

		if (m_player->GetState() != PlayerState::Attack)
		{
			float moveX = 0.0F;
			float moveY = 0.0F;

			if (M_INPUT->GetAction(eActionCode::MoveRight))
			{
				moveX += 1.0F;
			}

			if (M_INPUT->GetAction(eActionCode::MoveLeft))
			{
				moveX -= 1.0F;
			}

			if (M_INPUT->GetAction(eActionCode::MoveUp))
			{
				moveY -= 1.0F;
			}

			if (M_INPUT->GetAction(eActionCode::MoveDown))
			{
				moveY += 1.0F;
			}

			const float directionLength = std::sqrt(moveX * moveX +moveY * moveY);

			if (directionLength > 0.0F)
			{
				// 대각선 방향을 포함해 방향 벡터의 길이를 1로 만든다.
				moveX /= directionLength;
				moveY /= directionLength;

				const float movedDistance = moveSpeed * M_TIME->GetDeltaTime();

				pos.x += moveX * movedDistance;
				pos.y += moveY * movedDistance;

				if (moveX > 0.0F)
				{
					m_player->SetFacing(FacingDirection::Right);
					m_animator->SetFlipX(true);
				}
				else if (moveX < 0.0F)
				{
					m_player->SetFacing(FacingDirection::Left);
					m_animator->SetFlipX(false);
				}

				moved = true;

				tr->SetPosition(pos);

				if (m_player->GetPlayerLocation() != nullptr)
				{
					m_player->GetPlayerLocation()->pos = pos;
				}
			}
		}

		if (changeState)
		{
			stb::Player* player = M_PLAYERMANAGER->GetLocalPlayer();

			if (player != nullptr)
			{
				if (moved)
				{
					player->SetState(PlayerState::Walk);
				}
				else
				{
					player->SetState(PlayerState::Idle);
				}
			}
		}

		SyncFollowers(pos);

		if (moved)
		{
			mNetworkSendTimer += M_TIME->GetDeltaTime();

			if (mNetworkSendTimer >= NETWORK_SEND_INTERVAL)
			{
				auto networkManager = stb::NetworkManager::getInstance();

				if (networkManager != nullptr &&
					networkManager->IsConnected())
				{
					MovePacketHandler::SendPlayerMove(
						m_player
					);
				}

				mNetworkSendTimer = 0.0F;
			}
		}
		else
		{
			mNetworkSendTimer = 0.0F;
		}
	}

	void PlayerScript::Move()
	{

	}

	void PlayerScript::Attack(const eSkillCode skillCode)
	{
		stb::Player* player = m_player;

		if (player == nullptr)
			return;

		if (player->GetCombatSystem() == nullptr)
			return;

		if (player->GetState() >= PlayerState::Attack && player->GetState() < PlayerState::Skill_End)
			return;

		//기본공격
		if (skillCode == eSkillCode::None)
		{
			if (player->GetCombatSystem()->TryBasicAttack())
			{
				player->SetState(PlayerState::Attack);
				mAttackTimer = 0.0f;

				OutputDebugStringA("Player Attack Start\n");
			}
		}
		else //skill
		{
			if (player->GetCombatSystem()->TrySkillAttack((int)skillCode))
			{
				player->SetState(PlayerState::Skill_Slash);
				mAttackTimer = 0.0f;

				// 스킬 데이터 조회
				const SkillData* skillData = M_SKILLDATAMANAGER->FindItemData(static_cast<int>(skillCode));

				if (skillData != nullptr)
				{
					OutputDebugStringA(("[Skill VFX] " + skillData->skillVfx_name + "\n").c_str());
					Transform* tr = GetOwner()->GetComponent<Transform>();

					if (tr != nullptr)
					{
						Vector2 playerPos = tr->GetPosition();
						bool flipX = player->GetFacing() == FacingDirection::Right;
						M_SKILLEFFECTMANAGER->PlayCharge(skillData->skillVfx_name, playerPos, flipX);
						M_SKILLEFFECTMANAGER->PlayEffect(skillData->skillVfx_name, playerPos, flipX);
					}
				}

				OutputDebugStringA("Player Skill Attack Start\n");
			}
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
		{
			OutputDebugStringA("Transform is nullptr\n");
			return;
		}


		stb::Scene* scene = M_SCENEMANAGER->GetActiveScene();

		if (scene == nullptr)
		{
			OutputDebugStringA("scene is nullptr\n");
			return;
		}

		DropItemManager* dropManager = scene->GetDropItemManager();
		if (dropManager == nullptr)
			return;

		dropManager->RequestPickup(tr->GetPosition());
	}

	void PlayerScript::HandleInput()
	{
		//채팅 입력중 -> 단축키 차단
		if (M_UIMANAGER->IsGameplayInputBlocked())
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
		if (M_UIMANAGER->IsGameplayInputBlocked())
			return;

		if (M_INPUT->GetActionDown(eActionCode::Attack))
		{
			m_debugMsg = "HandleComabatInput is Pressed\n";
			OutputDebugStringA(m_debugMsg.c_str());
			Attack();
		}
		else if (M_INPUT->GetSkillDown(eSkillCode::Knight_Slash))
		{
			m_debugMsg = "HandleComabatInput is Pressed(Skill)\n";
			OutputDebugStringA(m_debugMsg.c_str());
			Attack(eSkillCode::Knight_Slash);
			//	OutputDebugStringA("Skill Execute\n");
		}
	}

	void PlayerScript::ExecuteBind(const KeyBindInfo& bindInfo)
	{
		switch (bindInfo.type)
		{
		case eBindType::Action:
			ExecuteAction((eActionCode)bindInfo.value);
			break;
		//case eBindType::Skill:
		//	// TODO : ��ų ��� ��û
		//	//SkillManager::GetInstance()->UseSkill(bindInfo.value);
		//	OutputDebugStringA("Skill Execute\n");
		//	break;
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
