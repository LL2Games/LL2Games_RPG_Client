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
	SetState(MonsterState::E_Move);
	BindAnimationEvents();
}

void Monster::Update(float deltaTime)
{
	GameObject::Update();

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

	switch (state)
	{
		case MonsterState::E_Idle : 
			/*m_currentAnimation = L"stand";
			M_LOGGER("MonsterState[idle]");
			break;*/
		case MonsterState::E_Move:
			m_currentAnimation = L"move";
			M_LOGGER("MonsterState[move]");
			break;
		case MonsterState::E_Hit:
			m_currentAnimation = L"hit";
			M_LOGGER("MonsterState[hit]");
			break;
		case MonsterState::E_Die:
			m_currentAnimation = L"die";
			M_LOGGER("MonsterState[die]");
			break;
	}

	stb::Animator* animator = GetComponent<stb::Animator>();
	if (animator != nullptr)
	{
		m_debugMsg ="current State : " + std::to_string(static_cast<int>(m_state)) + "\n";
		OutputDebugStringA(m_debugMsg.c_str());
		bool isLoop = false;
		if (m_currentAnimation != L"die") isLoop = true;
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
			if (m_state != MonsterState::E_Die)
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
	bool wasDead = (m_state == MonsterState::E_Die && m_isDeathAnimationFinished);
	m_targetPos = info.pos;

	m_dir = info.dir;
	//m_moveSpeed = info.moveSpeed;
	m_curHp = info.curHp;
	m_maxHp = info.maxHp;

	M_LOGGER("MonsterId:%d", info.monsterId);
	M_LOGGER("dir:%d", m_dir);
	M_LOGGER("targetPos.x:%f", m_targetPos.x);
	M_LOGGER("targetPos.y:%f", m_targetPos.y);
	if (wasDead && info.curHp > 0 && info.state != MonsterState::E_Die)
	{
		m_isDeathAnimationFinished = false;
		m_isDead = false;
		// 위치도 바로 스폰 위치로 맞추는 게 좋음
		m_transform->SetPosition(info.pos);
		m_targetPos = info.pos;

		// 이전 상태가 Die라서 SetState가 꼬이지 않게 강제 초기화
		m_state = MonsterState::E_NONE;
		SetState(MonsterState::E_Idle);

		return;
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
		// 죽었을 때 처리 해야함
		return;
	}

	SetState(MonsterState::E_Hit);
}

void Monster::RespawnFromServer(const MonsterUpdateInfo& info)
{
	m_isDeathAnimationFinished = false;
	m_isDead = false;

	m_curHp = info.curHp;
	m_maxHp = info.maxHp;

	m_pos= info.pos;
	m_targetPos = m_pos;

	SetState(MonsterState::E_Idle);
}

void Monster::ResetFromSpawnInfo(const MonsterSpawnInfo& info)
{
	
	m_pos = info.pos;
	std::string DebugMsg = "Monster Pos X : " + std::to_string(m_pos.x) + "\n" +
		"Monster Pos Y : " + std::to_string(m_pos.y) + "\n";

	OutputDebugStringA(DebugMsg.c_str());
	m_dir = info.dir;
	m_moveSpeed = info.moveSpeed;
	m_curHp = info.curHp;
	m_maxHp = info.maxHp;
	m_isDead = false;
	m_isDeathAnimationFinished = false; 

	if (m_transform != nullptr)
		m_transform->SetPosition(m_pos);
}
