#include "stbPlayer.h"
#include "InventoryManager.h"
#include "stbPlayerScript.h"
#include "..\\LL2_Client_Win_Source\\\PlayerAnimationManager.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbRender.h"
#include "stbCamera.h"
#include "StringConvert.h"
#include "stbD2DRenderer.h"


#define M_PLAYERANIMMANAGER stb::SingletonBase<PlayerAnimationManager>::getInstance()
#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()

namespace stb
{
	Player::Player() : m_inven(InventoryManager::getInstance()), m_playerState(PlayerState::None),
					   m_transform(nullptr), m_animator(nullptr), m_damageText(nullptr), m_collider(nullptr), m_script(nullptr), m_isLocalPlayer(false)
	{
		m_combatSystem.SetOwner(this);
	}
	void Player::Initialize()
	{
		GameObject::Initialize();
		m_transform = AddComponent<stb::Transform>();
		m_animator = AddComponent<stb::Animator>();
		m_damageText = AddComponent<stb::DamageText>();
		m_collider = AddComponent<stb::BoxCollider2D>();
		m_script = AddComponent<PlayerScript>();
		m_quickSlotManager.Init();

		//m_quickSlotManager.SetOwner(this);
		m_script->SetOwner(this);
		m_script->SetPlayer(this);
		m_script->SetAnimator();
		
	}

	void Player::Update()
	{
		GameObject::Update();
	}

	void Player::LateUpdate()
	{
		GameObject::LateUpdate();
	}

	void Player::Render(HDC hdc)
	{
		GameObject::Render(hdc);
	}

	void Player::Render(stbD2DRenderer& renderer)
	{
		GameObject::Render(renderer);

		if (m_transform == nullptr ||
			m_playerProfile.name.empty())
		{
			return;
		}

		Vector2 screenPos = m_transform->GetPosition();

		if (render::mainCamera != nullptr)
		{
			screenPos =
				render::mainCamera->CalculatePosition(screenPos);
		}

		D2D1_RECT_F nameRect = D2D1::RectF(
			screenPos.x - 60.0f,
			screenPos.y - 85.0f,
			screenPos.x + 60.0f,
			screenPos.y - 60.0f
		);

		std::wstring name =
			Convert::StringToWString(m_playerProfile.name);

		// 그림자
		D2D1_RECT_F shadowRect = nameRect;
		shadowRect.left += 1.0f;
		shadowRect.right += 1.0f;
		shadowRect.top += 1.0f;
		shadowRect.bottom += 1.0f;

		renderer.DrawTextString(
			name,
			shadowRect,
			D2D1::ColorF(D2D1::ColorF::Black),
			TextStyle::NickName
		);

		// 본문
		renderer.DrawTextString(
			name,
			nameRect,
			D2D1::ColorF(D2D1::ColorF::White),
			TextStyle::NickName
		);
	}

	void Player::SetStat(BaseStat baseStat, DerivedStat derived, ExpStat expStat, int cur_hp, int cur_mp, int remainAp)
	{
		m_stat.SetStat(baseStat, derived, expStat, cur_hp, cur_mp, remainAp);
	}

	void Player::SetPlayerInfo(PlayerIdentity playeridentity, PlayerProfile playerProfile, PlayerLocation playerlocation)
	{
		m_playerIdentity = playeridentity;
		m_playerProfile = playerProfile;
		m_playerLocation = playerlocation;

		if (m_transform != nullptr)
		{
			m_transform->SetPosition(m_playerLocation.pos);
		}

		JobType jobType = static_cast<JobType>(m_playerProfile.job);
		WeaponType weaponType = WeaponType::One_Hand;

		M_PLAYERANIMMANAGER->Init();

		bool setupOk = M_PLAYERANIMMANAGER->SetupPlayerAnimations(this, jobType, weaponType);
		if (!setupOk)
		{
			stb::Texture* knightTex = M_REMANAGER->Find<stb::Texture>(L"DamonKnight");
			if (knightTex != nullptr && m_animator != nullptr)
			{
				m_animator->CreateAnimation(L"stand", knightTex, Vector2(0.0f, 0.0f), Vector2(67.0f, 81.0f), Vector2::Zero, 1, 1.0f);
				m_animator->CreateAnimation(L"walk", knightTex, Vector2(0.0f, 0.0f), Vector2(67.0f, 81.0f), Vector2::Zero, 3, 0.3f);
				m_animator->CreateAnimation(L"swingO3", knightTex, Vector2(0.0f, 0.0f), Vector2(67.0f, 81.0f), Vector2::Zero, 3, 0.2f);
			}
		}
		m_playerState = PlayerState::None;
		SetState(PlayerState::Idle);
	}

	void Player::PlayAttackAnimation(int skillId)
	{


	}

	void Player::SetState(PlayerState state)
	{
		if (m_playerState == state)
			return;

		m_playerState = state;

		switch (state)
		{
		case PlayerState::Idle:
			m_currentAnimation = L"stand";
			break;

		case PlayerState::Walk:
			m_currentAnimation = L"walk";
			break;

		case PlayerState::Attack:
			m_currentAnimation = L"swingO3";
			break;
		}

		stb::Animator* animator = GetComponent<stb::Animator>();
		if (animator != nullptr)
		{
			bool isLoop = true;

			if (state == PlayerState::Attack)
				isLoop = false;

			animator->PlayAnimation(m_currentAnimation, isLoop);
		}
	}

	void Player::OnDamaged(int damage, int curHp)
	{
		m_stat.SetCurHp(curHp);
		ShowDamageText(damage);

		// 필요하면 여기서 피격 상태/무적/넉백도 나중에 추가
	}

	void Player::ShowDamageText(int damage)
	{
		if (m_damageText != nullptr)
			m_damageText->AddDamage(damage);
	}

}

