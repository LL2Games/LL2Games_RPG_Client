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
#include "MovementPacketHandler.h"
#include "MapDataManager.h"
#include "GameUiMessages.h"
#include "SkillDataManager.h"
#include "SkillEffectManager.h"

#include <cmath>

#include "stbCamera.h"
#include "stbRender.h"
#include "stbApplication.h"
#include "HealthBarUI.h"
#include <algorithm>


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
		: mHead(nullptr)
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
        if (!m_player || !m_animator) return;
        const float dt = M_TIME->GetDeltaTime();
        auto* network = stb::NetworkManager::getInstance();
        if (!network || !network->IsConnected())
        {
            ResetMovementConnection();
            return;
        }
        movement::Snapshot displayed;
        if (m_movement.Update(dt, displayed))
        {
            if (auto* tr = m_player->GetComponent<Transform>())
                tr->SetPosition({displayed.position.x, displayed.position.y});
            m_player->GetPlayerLocation()->pos = {displayed.position.x, displayed.position.y};
            m_player->SetFacing(displayed.facing > 0 ? FacingDirection::Right : FacingDirection::Left);
            SyncFollowers({displayed.position.x, displayed.position.y});
            const auto* map = MapDataManager::getInstance()->FindMapData(displayed.mapId);
            const auto* climb = map ? map->physics.FindClimbable(displayed.climbableId) : nullptr;
            const bool attacking = m_player->GetState() >= PlayerState::Attack &&
                                   m_player->GetState() < PlayerState::Skill_End;
            m_movementVisual.Update(m_animator, displayed, dt, climb && climb->ladder,
                attacking && displayed.mode != movement::Mode::Climbing && !movement::IsStunned(displayed));
        }
        const HWND window = stb::Application::getInstance()->GetHWND();
        const auto held = [](eActionCode action)
        {
            return M_INPUT->GetAction(action) || M_INPUT->GetActionDown(action);
        };
        bool blocked = GetForegroundWindow() != GetAncestor(window, GA_ROOT) ||
            M_UIMANAGER->IsInputFocused() || M_UIMANAGER->IsGameplayInputBlocked() || !CanMove();
        if (blocked) { m_jumpPending = false; m_jumpNeedsRelease = true; }
        else
        {
            if (!held(eActionCode::Jump)) m_jumpNeedsRelease = false;
            if (CanAttack()) HandleCombatInput();
            HandleInput();
            blocked = M_UIMANAGER->IsInputFocused() || M_UIMANAGER->IsGameplayInputBlocked();
            if (!m_jumpNeedsRelease && M_INPUT->GetActionDown(eActionCode::Jump)) m_jumpPending = true;
        }
        if (blocked) { m_jumpPending = false; m_jumpNeedsRelease = true; }
        movement::Input input;
        if (!blocked)
        {
            input.horizontal = int(held(eActionCode::MoveRight)) - int(held(eActionCode::MoveLeft));
            input.vertical = int(held(eActionCode::MoveDown)) - int(held(eActionCode::MoveUp));
            input.jump = m_jumpPending;
        }
        int sequence = 0;
        if (CanMove() && m_inputSchedule.Poll(input, dt, blocked != m_inputBlocked, sequence))
        {
            const auto& latest = m_movement.Latest();
            MovementPacketHandler::SendInput(latest.mapId, latest.epoch, sequence, input);
        }
        m_inputBlocked = blocked;
        m_jumpPending = false;
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

	void PlayerScript::Attack(const eSkillCode skillCode)
	{
        if (!CanAttack()) return;
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
        if (CanMove() && !m_jumpNeedsRelease) m_jumpPending = true;
    }

    bool PlayerScript::CanMove() const
    {
        return m_movement.Ready() && !movement::IsDead(m_movement.Latest()) &&
            !movement::IsStunned(m_movement.Latest()) && m_player && !m_player->IsDead();
    }
    bool PlayerScript::CanAttack() const
    {
        return CanMove() && m_movement.Latest().mode != movement::Mode::Climbing;
    }
    bool PlayerScript::CanUsePortal() const
    {
        return CanMove() && m_movement.Latest().mode == movement::Mode::Grounded;
    }
    void PlayerScript::StopMovementInput()
    {
        if (CanMove())
        {
            int sequence = 0;
            if (m_inputSchedule.Poll({}, 0, true, sequence))
            {
                const auto& s = m_movement.Latest();
                MovementPacketHandler::SendInput(s.mapId, s.epoch, sequence, {});
            }
        }
        m_inputBlocked = true; m_jumpPending = false; m_jumpNeedsRelease = true;
    }
    void PlayerScript::SuspendMovement()
    {
        StopMovementInput();
        m_movement.Suspend(); m_movementVisual.Reset();
        if (m_animator) m_animator->SetPaused(false);
    }
    void PlayerScript::CancelMovementSuspend()
    {
        m_movement.CancelSuspend(); m_inputBlocked = true;
    }
    void PlayerScript::ResetMovementConnection()
    {
        m_movement.Reset(); m_inputSchedule.Reset(); m_movementVisual.Reset();
        m_inputBlocked = true; m_jumpPending = false; m_jumpNeedsRelease = true;
        mAttackTimer = 0;
        if (m_animator) m_animator->SetPaused(false);
    }
    void PlayerScript::OnServerDeath()
    {
        if (!m_player) return;
        const bool firstDeath = !m_player->IsDead();
        StopMovementInput();
        m_movement.Suspend(true); m_movementVisual.Reset();
        m_player->SetState(PlayerState::Dead);
        if (firstDeath)
            ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_SHOW_REVIVE, 0, 0);
    }
    void PlayerScript::ApplyMovementSnapshot(const movement::Snapshot& s)
    {
        if (!m_player || s.mapId != m_player->GetPlayerLocation()->mapId) return;
        const bool newEpoch = !m_movement.HasSnapshot() || s.epoch != m_movement.Latest().epoch;
        const bool wasDead = m_player->IsDead();
        const int oldLife = m_movement.HasSnapshot() ? m_movement.Latest().lifeState : -1;
        if (!m_movement.Push(s)) return;
        if (newEpoch)
        {
            m_inputSchedule.Reset(s.sequence); m_movementVisual.Reset();
            m_jumpPending = false; m_jumpNeedsRelease = true; m_inputBlocked = true;
            m_player->SetState(movement::IsDead(s) ? PlayerState::Dead : PlayerState::Idle);
        }
        m_player->GetStat()->SetHealth(s.hp, s.maxHp);
        if (!movement::IsDead(s) && (s.mode == movement::Mode::Climbing || movement::IsStunned(s)))
            m_player->SetState(PlayerState::Idle);
        if (movement::IsDead(s))
        {
            m_jumpPending = false; m_jumpNeedsRelease = true;
            m_player->SetState(PlayerState::Dead);
            if (!wasDead) ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_SHOW_REVIVE, 0, 0);
        }
        else if (wasDead && newEpoch)
        {
            m_player->SetState(PlayerState::Idle);
            ::PostMessageW(stb::Application::getInstance()->GetHWND(), WM_REVIVE_SUCCESS, 0, 0);
        }
        else if (s.lifeState == static_cast<int>(movement::PlayerLife::Attack) && oldLife != s.lifeState &&
                 !(m_player->GetState() >= PlayerState::Attack && m_player->GetState() < PlayerState::Skill_End))
            m_player->SetState(PlayerState::Attack);
        if (newEpoch || movement::IsDead(s))
        {
            if (auto* tr = m_player->GetComponent<Transform>()) tr->SetPosition({s.position.x, s.position.y});
            m_player->GetPlayerLocation()->pos = {s.position.x, s.position.y};
            SyncFollowers({s.position.x, s.position.y});
        }
#ifdef _DEBUG
        OutputDebugStringA(("[Movement self] map=" + std::to_string(s.mapId) + " epoch=" + std::to_string(s.epoch) +
            " sequence=" + std::to_string(s.sequence) + " tick=" + std::to_string(s.tick) + "\n").c_str());
#endif
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

}
