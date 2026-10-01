#include "MovementDebug.h"
#include "Monster.h"
#include "stbResourceManager.h"
#include "MonsterDataManager.h"
#include "MapDataManager.h"
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
    movement::Snapshot displayed;
    if (!m_movement.Update(deltaTime, displayed)) return;
    m_pos = {displayed.position.x, displayed.position.y};
    m_transform->SetPosition(m_pos);
    m_dir = displayed.facing;
    const auto* map = MapDataManager::getInstance()->FindMapData(displayed.mapId);
    const auto* climb = map ? map->physics.FindClimbable(displayed.climbableId) : nullptr;
    m_movementVisual.Update(m_animator, displayed, deltaTime, climb && climb->ladder,
        m_state == MonsterState::E_Hit || m_state == MonsterState::E_Die);
}
// 병합 시 수정 부분인지 확인 불가로 남겨둠
  /*
	if (m_state == MonsterState::E_Die || m_state == MonsterState::E_Dead)
		return;

	stb::math::Vector2 diff = m_targetPos - m_pos;
	float dist = diff.length();

	if (dist > 1.0f)
	{
		float correctionSpeed = static_cast<float>(m_moveSpeed);
		//float correctionSpeed = m_moveSpeed * 2.0f;
		float moveDist = correctionSpeed * deltaTime;
    */

void Monster::ApplyMovementSnapshot(const movement::Snapshot& s)
{
    const bool reset = !m_movement.HasSnapshot() || s.epoch != m_movement.Latest().epoch;
    const int oldLife = m_movement.HasSnapshot() ? m_movement.Latest().lifeState : -1;
    if (!m_movement.Push(s)) return;
    m_curHp = s.hp; m_maxHp = s.maxHp;
    if (reset)
    {
        m_movementVisual.Reset();
        m_pos = m_targetPos = {s.position.x, s.position.y};
        m_transform->SetPosition(m_pos);
        m_isDead = false; m_isDeathAnimationFinished = false;
        m_state = MonsterState::E_NONE;
    }
    if (s.lifeState == static_cast<int>(movement::MonsterLife::Dead))
    {
        SetState(MonsterState::E_Die);
        m_isDead = true; m_isDeathAnimationFinished = true;
    }
    else if (movement::IsDead(s)) SetState(MonsterState::E_Die);
    else if (reset || oldLife != s.lifeState)
    {
        switch (static_cast<movement::MonsterLife>(s.lifeState))
        {
        case movement::MonsterLife::Hit: SetState(MonsterState::E_Hit); break;
        case movement::MonsterLife::Patrol: SetState(MonsterState::E_Patrol); break;
        case movement::MonsterLife::Chase: SetState(MonsterState::E_Chase); break;
        case movement::MonsterLife::Move: SetState(MonsterState::E_Move); break;
        default: SetState(MonsterState::E_Idle); break;
        }
    }
}

float Monster::GetFootOffset() const
{
    const auto* data = M_MONSTERDATAMANAGER->FindMonsterData(m_monsterId);
    if (!data) return 0;
    return data->colliderInfo.offset.y +
        (data->colliderInfo.colliderType == stb::enums::eColliderType::Circle2D
            ? data->colliderInfo.radius : data->colliderInfo.halfSize.y);
}

void Monster::Render(stbD2DRenderer& renderer)
{
	if ((m_state == MonsterState::E_Die || m_state == MonsterState::E_Dead)
		&& m_isDeathAnimationFinished)
		return;


	GameObject::Render(renderer);
        if (m_transform) movement::DrawOriginAndFeet(renderer, m_transform->GetPosition(), GetFootOffset());
	//m_collider->Render(renderer);
}

void Monster::SetState(MonsterState state)
{
	if (m_state == state)
		return;

	m_state = state;
    if (m_animator) m_animator->SetPaused(false);

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
		case MonsterState::E_RangeAttack:
			m_currentAnimation = L"attack";
			break;
		case MonsterState::E_Die:
		case MonsterState::E_Dead:
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
	
		if (state == MonsterState::E_Hit
			|| state == MonsterState::E_Die
			|| state == MonsterState::E_Dead
			|| state == MonsterState::E_RangeAttack)
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
			static_cast<float>(info.delay_ms) / 1000.0f
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

	m_animator->RegisterEvent(L"MonsterAttackEnd", [this]()
		{
			if (m_state == MonsterState::E_RangeAttack)
				SetState(MonsterState::E_Idle);
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
    if (m_movement.HasSnapshot()) return;
	m_targetPos = info.pos;
	m_dir = info.dir;

	m_curHp = info.curHp;
	m_maxHp = info.maxHp;

	// 죽음 애니메이션은 Respawn 패킷이 올 때까지 유지
	if (m_state == MonsterState::E_Die || m_state == MonsterState::E_Dead)
		return;

	// 피격 애니메이션은 끝날 때까지 유지
	// MonsterHitEnd에서 Idle로 변경한다.
	if (m_state == MonsterState::E_Hit)
		return;

	if (m_monsterId == 100200)
	{
		if (info.state == MonsterState::E_RangeAttack)
		{
			if (m_bossAttackStateActive)
				return; // 같은 공격 상태의 반복 패킷은 애니메이션을 재시작하지 않음

			m_bossAttackStateActive = true;
		}
		else
		{
			m_bossAttackStateActive = false;
		}
	}


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
    // A new live snapshot may arrive before the legacy respawn notification.
    if (m_movement.HasSnapshot() && !movement::IsDead(m_movement.Latest())) return;
    m_movement.Suspend();
    m_movementVisual.Reset();
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
    if (m_movement.HasSnapshot()) return;
	
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
