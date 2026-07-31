#pragma once
#include "..\\LL2_Client_Win_Source\\stbScript.h"
#include "..\\LL2_Client_Win_Source\\\CombatSystem.h"
#include "..\\LL2_Client_Win_Source\\stbInput.h"
#include "..\\LL2_Client_Win_Source\\stbAnimation.h"
#include "..\\LL2_Client_Win_Source\\QuickSlotManager.h"

namespace stb { class Player; }
class stbD2DRenderer;

namespace stb
{
	class PlayerScript : public Script
	{
	public:
		PlayerScript();
		~PlayerScript();

		void Initialize() override;
		void Update() override;
		
		void LateUpdate() override;
		void Render(HDC hdc) override;
	
	 
	public:


		void SetFollowers(GameObject* head, GameObject* sword)
		{
			mHead = head;
			mSword = sword;
		}

		void SetPlayer(stb::Player* player) { m_player = player; }
		void SetAnimator();
	private:
		void UpdateAttackState();
		void Idle(bool changeState = true);
		void Move();
		void Attack(const eSkillCode skillCode = eSkillCode::None);
		void Jump();
		void PickUp();
		void HandleInput();
		void HandleCombatInput();
		void ExecuteBind(const KeyBindInfo& bindInfo);
		void ExecuteAction(eActionCode action);
		void SyncFollowers(Vector2 pos);

		bool IsMoveInputPressed() const;
	
	private:
		float mNetworkSendTimer;
		const float NETWORK_SEND_INTERVAL = 0.03f;
		float mAttackTimer;
		float mAttackDuration;
		GameObject* mHead;
		GameObject* mSword;

		stb::Player* m_player;
		stb::Animator* m_animator;

		std::string m_debugMsg;
		QuickSlotManager* m_quickSlotManager;
	};
}


