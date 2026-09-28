#include "Monster.h"
#include "stbResourceManager.h"
#include "MonsterDataManager.h"
#include "stbTexture.h"
#include "Util.h"
#include "stbLogger.h"

#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_MONSTERDATAMANAGER stb::SingletonBase<MonsterDataManager>::getInstance()

void Monster::Initialize()
{
	GameObject::Initialize();

	m_transform = AddComponent<stb::Transform>();
	m_animator = AddComponent<stb::Animator>();
	m_script = AddComponent<MonsterScript>();

	m_script->SetOwner(this);
}


void Monster::InitFromSpawn(const MonsterSpawnInfo& info)
{
	m_instanceId = info.instanceId;
	m_monsterId = info.monsterId;

	ResetFromSpawnInfo(info);

	SetAnimation();
	SetCollider();
	BindAnimationEvents();

	// 최초 애니메이션을 무조건 재생시키기 위해
	m_state = MonsterState::E_NONE;

	SetState(info.state);
}

void Monster::Update(float deltaTime)
{
	GameObject::Update();

	if (m_state == MonsterState::E_Die)
		return;

	stb::math::Vector2 diff = m_targetPos - m_pos;
	float dist = diff.length();

	//DebugMsg = "dist : " + std::to_string(dist) + "\n";
	//OutputDebugStringA(DebugMsg.c_str());

	if (dist > 1.0f)
	{
		float correctionSpeed = static_cast<float>(m_moveSpeed);
		//float correctionSpeed = m_moveSpeed * 2.0f;
		float moveDist = correctionSpeed * deltaTime;

		if (moveDist >= dist)
			m_pos = m_targetPos;
		else
			m_pos += diff.normalize() * moveDist;

		m_transform->SetPosition(m_pos);
	}
}

void Monster::Render(stbD2DRenderer& renderer)
{
	if (m_state == MonsterState::E_Die && m_isDeathAnimationFinished)
		return;


	GameObject::Render(renderer);
	//m_collider->Render(renderer);
}

void Monster::SetState(MonsterState state)
{
	if (m_state == state)
		return;

	m_state = state;

	bool isLoop = true;

	switch (state)
	{
		case MonsterState::E_Idle : 
			m_currentAnimation = L"idle";
			M_LOGGER("MonsterState[idle]");
			break;
		case MonsterState::E_Patrol:
		case MonsterState::E_Chase:
		case MonsterState::E_Move:
			m_currentAnimation = L"move";
			break;
		case MonsterState::E_Hit:
			m_currentAnimation = L"hit";
			M_LOGGER("MonsterState[hit]");
			break;
		case MonsterState::E_Die:
			m_currentAnimation = L"die";
			M_LOGGER("MonsterState[die]");
			break;
		default:
			return;
	}

	stb::Animator* animator = GetComponent<stb::Animator>();
	if (animator != nullptr)
	{
		m_debugMsg ="current State : " + std::to_string(static_cast<int>(m_state)) + "\n";
		OutputDebugStringA(m_debugMsg.c_str());
	
		if (state == MonsterState::E_Hit || state == MonsterState::E_Die)
		{
			isLoop = false;
		}
		animator->PlayAnimation(m_currentAnimation, isLoop);
		OutputDebugStringA("Monster PlayAnimation \n");
	}


}
void Monster::SetPosition(float /*x*/, float /*y*/)
{

}

void Monster::SetAnimation()
{
	const MonsterData* data = M_MONSTERDATAMANAGER->FindMonsterData(m_monsterId);
	if (data == nullptr) 
	{
		std::string DebugMsg = "MonsterData is nullptr \n";
		OutputDebugStringA(DebugMsg.c_str());
		return;
	}
		
	for (const AnimationInfo& info : data->animations)
	{
		std::vector<stb::Texture*> frames;

		for (int i = 0; i < info.frame_count; ++i)
		{
			std::wstring key =
				utils::StringToWString(info.path + "/" + info.frame_prefix + std::to_string(i));
			
			stb::Texture* tex = M_RESOURCEMANAGER->Find<stb::Texture>(key);
			if (tex != nullptr)
				frames.emplace_back(tex);
		}
	
		m_animator->CreateFrameAnimation(
			utils::StringToWString(info.anim_name),
			frames,
			data->renderInfo.origin,
			data->renderInfo.offset,
			0.2f
		);

		stb::Animator::EventNames eventNames;

		eventNames.startEventName = utils::StringToWString(info.animationEvent.start);
		eventNames.completeEventName = utils::StringToWString(info.animationEvent.complete);
		eventNames.endEventName = utils::StringToWString(info.animationEvent.end);

		m_animator->SetAnimationEventNames(utils::StringToWString(info.anim_name), eventNames);

	}
}
void Monster::SetCollider()
{
	const MonsterData* data = M_MONSTERDATAMANAGER->FindMonsterData(m_monsterId);
	if (data == nullptr)
	{
		std::string DebugMsg = "MonsterData is nullptr \n";
		OutputDebugStringA(DebugMsg.c_str());
		return;
	}

	if (data->colliderInfo.colliderType == stb::eColliderType::Rect2D)
	{
		m_collider = AddComponent<stb::BoxCollider2D>();
	}	
	else 
	{
		m_collider = AddComponent<stb::CircleCollider2D>();
	}
	
	m_collider->SetOffset(data->colliderInfo.offset);
	m_collider->SetSize(data->colliderInfo.halfSize);

}

void Monster::BindAnimationEvents()
{
	m_animator->RegisterEvent(L"MonsterHitEnd", [this]()
		{
			OutputDebugStringA("MonsterHitEnd event called\n");
			if (m_state == MonsterState::E_Hit)
			{
				SetState(MonsterState::E_Idle);
			}
		});

	m_animator->RegisterEvent(L"MonsterDieEnd", [this]()
		{
			OutputDebugStringA("MonsterDieEnd event called\n");
			m_isDeathAnimationFinished = true;
			m_isDead = true;

		});
}
void Monster::OnDamaged(int /*damage*/, int /*curHp*/, bool /*dead*/)
{

}
void Monster::OnMove(float /*x*/, float /*y*/, int /*dir*/)
{
	
}

#include "stbLogger.h"
// 몬스터패킷 핸들러에서 바로 호출하는 함수
void Monster::ApplyServerUpdate(const MonsterUpdateInfo& info)
{
	m_targetPos = info.pos;
	m_dir = info.dir;

	m_curHp = info.curHp;
	m_maxHp = info.maxHp;

	// 죽음 애니메이션은 Respawn 패킷이 올 때까지 유지
	if (m_state == MonsterState::E_Die)
		return;

	// 피격 애니메이션은 끝날 때까지 유지
	// MonsterHitEnd에서 Idle로 변경한다.
	if (m_state == MonsterState::E_Hit)
		return;

	SetState(info.state);
}

void Monster::ApplyAttackResult(const AttackResult& result)
{
	m_curHp = result.cur_hp;
	m_maxHp = result.max_hp;

	if (result.isDead)
	{
		SetState(MonsterState::E_Die);
		// 죽었을 때 처리 해야함SetState
		return;
	}

	SetState(MonsterState::E_Hit);
}

void Monster::RespawnFromServer(const MonsterUpdateInfo& info)
{
	OutputDebugStringA("[RESPAWN] RespawnFromServer called\n");

	m_isDeathAnimationFinished = false;
	m_isDead = false;

	m_curHp = info.curHp;
	m_maxHp = info.maxHp;

	m_pos= info.pos;
	m_targetPos = m_pos;

	if (m_transform != nullptr)
		m_transform->SetPosition(info.pos);

	// Animator까지 확실하게 다시 시작
	m_state = MonsterState::E_NONE;

	OutputDebugStringA("[RESPAWN] SetState Idle\n");

	SetState(MonsterState::E_Idle);
}

void Monster::ResetFromSpawnInfo(const MonsterSpawnInfo& info)
{
	
	m_pos = info.pos;

	m_dir = info.dir;
	m_moveSpeed = info.moveSpeed;
	m_curHp = info.curHp;
	m_maxHp = info.maxHp;

	m_isDead = false;
	m_isDeathAnimationFinished = false;

	m_targetPos = info.pos;

	if (m_transform != nullptr)
		m_transform->SetPosition(m_pos);
}
