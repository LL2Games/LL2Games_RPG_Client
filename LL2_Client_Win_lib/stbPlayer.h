#pragma once
#include "..\\LL2_Client_Win_Source\\stbGameObject.h"
#include "..\\LL2_Client_Win_Source\\stbTransform.h"
#include "..\\LL2_Client_Win_Source\\stbAnimator.h"
#include "..\\LL2_Client_Win_Source\\stbDamageText.h"
#include "..\\LL2_Client_Win_Source\\BoxCollider2D.h"
#include "..\\LL2_Client_Win_Source\\Stat.h"
#include "..\\LL2_Client_Win_Source\\InventoryManager.h"
#include "..\\LL2_Client_Win_Source\\CombatSystem.h"
#include "..\\LL2_Client_Win_Source\\QuickSlotManager.h"
#include "stbPlayerScript.h"
#include "EquipeTypes.h"



class stbD2DRenderer;
namespace stb
{

	enum class FacingDirection
	{
		Left = -1,
		Right = 1
	};



	class Player : public GameObject
	{
	public:
		Player();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(HDC hdc) override;
		void Render(stbD2DRenderer& renderer);

		void SetStat(BaseStat baseStat, DerivedStat derived, ExpStat expStat, int cur_hp, int cur_mp, int remainAp);
		void SetPlayerInfo(PlayerIdentity playeridentity, PlayerProfile playerProfile, PlayerLocation playerlocation);
		void PlayAttackAnimation(int skillId);
	public:
		Stat* GetStat() { return &m_stat; }
		InventoryManager* GetInvenManager() { return m_inven; }

		const std::string GetPlayerName() { return m_playerProfile.name; }

		const PlayerIdentity* GetPlayerIdentity() const { return &m_playerIdentity; }
		PlayerIdentity* GetPlayerIdentity() { return &m_playerIdentity; }
		
		const PlayerProfile* GetPlayerProfile() const { return &m_playerProfile; }
		PlayerProfile* GetPlayerProfile() { return &m_playerProfile; }

		PlayerLocation* GetPlayerLocation() { return &m_playerLocation; }

		CombatSystem* GetCombatSystem() { return &m_combatSystem; }

		// 플레이어의 상태가 Dead인지 확인하는 함수
		bool IsDead() { return m_playerState == PlayerState::Dead; };

		// 플레이어의 상태가 Attack인지 확인하는 함수
		bool IsAttacking() { return m_playerState == PlayerState::Attack; }
		PlayerState GetState() { return m_playerState; }
		void SetState(PlayerState state);

		QuickSlotManager* GetQuickSlotManager() { return &m_quickSlotManager; }

		void SetFacing(FacingDirection facing) { m_facing = facing; }
		FacingDirection GetFacing() { return m_facing; }
		
		int GetWeaponTypeToInt() { return static_cast<int>(m_weaponType); }
		WeaponType GetWeaponType() { return m_weaponType; }

		void SetLocalPlayer(bool isLocalPlayer) { m_isLocalPlayer = isLocalPlayer; }
		bool IsLocalPlayer() { return m_isLocalPlayer; }

		Animator* GetAnimator() { return m_animator; }

		float GetPlayerMoveSpeed() { return m_moveSpeed; }

	public:
		void OnDamaged(int damage, int curHp);
		void ShowDamageText(int damage);

	private:
		WeaponType m_weaponType = WeaponType::None;
		
	private:
		// 
		Stat m_stat;
		InventoryManager* m_inven;

		// 플레이어 계정 정보
		PlayerIdentity m_playerIdentity;
		// 플레이어 프로필 정보
		PlayerProfile m_playerProfile;
		// 플레이어 위치 정보
		PlayerLocation m_playerLocation;

		// 플레이어 상태
		PlayerState m_playerState;

		// 플레이어 전투 시스템
		CombatSystem  m_combatSystem;

		QuickSlotManager m_quickSlotManager;
		
		FacingDirection m_facing = FacingDirection::Right;
	
	private:
		Transform* m_transform;
		Animator* m_animator;
		DamageText* m_damageText;
		BoxCollider2D* m_collider;
		PlayerScript* m_script;
		float m_moveSpeed;

		std::wstring m_currentAnimation;
		
		bool m_isLocalPlayer;

		std::string DebugMsg;

	};

}

