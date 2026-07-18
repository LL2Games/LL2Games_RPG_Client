#include "Projectile.h"


#include "stbResourceManager.h"
#include "MonsterDataManager.h"
#include "stbTexture.h"
#include "Util.h"

#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_MONSTERDATAMANAGER stb::SingletonBase<MonsterDataManager>::getInstance()


void Projectile::Initialize()
{
    GameObject::Initialize();

    m_transform = AddComponent<stb::Transform>();
    m_animator = AddComponent<stb::Animator>();
	m_travelled = 0.0f;
	//m_script->SetOwner(this);
}

void Projectile::InitFromServer(const MonsterProjectileData& info)
{
	m_instanceId = info.instanceId;
	m_projectileTypeId = info.projectileId;
	m_ownerId = info.ownerId;
	m_range = info.range;
	m_speed = info.speed;
	//m_moveSpeed = info.moveSpeed;

	m_position = info.pos;
	m_targetPosition = info.pos;
	m_direction = { info.dirX, info.dirY };

	if (m_transform != nullptr)
	{
		m_transform->SetPosition(m_position);
	}

	SetAnimation();
	
	if (m_animator != nullptr)
	{
		m_animator->PlayAnimation(L"projectile", true);
	}

	/*ResetFromSpawnInfo(info);
	SetAnimation();
	SetCollider();
	SetState(MonsterState::E_Move);
	BindAnimationEvents();*/
}

void Projectile::ApplyServerUpdate(const MonsterProjectileData& info)
{
	m_targetPosition = info.pos;
	m_direction = { info.dirX, info.dirY };

	//우선 출력 확인 위해 즉시 반영
	/*m_position = info.pos;

	if (m_transform != nullptr)
	{
		m_transform->SetPosition(m_position);
	}*/
}

static float Length(const stb::math::Vector2& v)
{
	return std::sqrt(v.x * v.x + v.y * v.y);
}

void Projectile::Update(float deltaTime)
{
	GameObject::Update();

	stb::math::Vector2 delta;
	delta.x = m_direction.x * m_speed * deltaTime;
	delta.y = m_direction.y * m_speed * deltaTime;

	m_position += delta;
	m_travelled += Length(delta);

	if (m_transform != nullptr)
		m_transform->SetPosition(m_position);

	////constexpr float projectileSpeed = 500.0f;
	//float projectileSpeed = m_speed;
	//constexpr float correctionStrength = 8.0f;

	//// 서버 패킷 사이에도 계속 움직이도록 로컬 예측
	//if (m_direction.length() > 0.0f)
	//{
	//	m_position += m_direction.normalize()
	//		* projectileSpeed
	//		* deltaTime;
	//}

	//// 서버 위치와 오차를 부드럽게 수정
	//stb::math::Vector2 error =
	//	m_targetPosition - m_position;

	//float correctionRatio = std::clamp(
	//	correctionStrength * deltaTime,
	//	0.0f,
	//	1.0f
	//);

	//m_position += error * correctionRatio;

	//if (m_transform != nullptr)
	//	m_transform->SetPosition(m_position);
}
//
//void Projectile::Update(float deltaTime)
//{
//	GameObject::Update();
//
//	stb::math::Vector2 diff = m_targetPosition - m_position;
//	float distance = diff.length();
//
//	if (distance > 0.1f)
//	{
//		//값이 높을수록 서버 위치를 빠르게 따라감
//		constexpr float correctionSpeed = 20.0f;
//		//constexpr float correctionSpeed = m_moveSpeed;
//
//		float ratio = std::clamp(
//			correctionSpeed * deltaTime,
//			0.0f,
//			1.0f
//		);
//
//		m_position += diff * ratio;
//	}
//	else
//	{
//		m_position = m_targetPosition;
//	}
//
//	if (m_transform != nullptr)
//		m_transform->SetPosition(m_position);
//
//	//stb::math::Vector2 diff = m_targetPos - m_pos;
//	//float dist = diff.length();
//
//	//if (dist > 1.0f)
//	//{
//	//	float correctionSpeed = static_cast<float>(m_moveSpeed);
//	//	//float correctionSpeed = m_moveSpeed * 2.0f;
//	//	float moveDist = correctionSpeed * deltaTime;
//
//	//	if (moveDist >= dist)
//	//		m_pos = m_targetPos;
//	//	else
//	//		m_pos += diff.normalize() * moveDist;
//
//	//	m_transform->SetPosition(m_pos);
//	//}
//}

void Projectile::Render(stbD2DRenderer& renderer)
{
	/*if (m_state == MonsterState::E_Die && m_isDeathAnimationFinished)
		return;*/


	GameObject::Render(renderer);
	//m_collider->Render(renderer);
}


void Projectile::SetAnimation()
{
	const MonsterData* data = M_MONSTERDATAMANAGER->FindMonsterData(m_ownerId);
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

bool Projectile::IsExpired()
{
	return m_travelled >= m_range;
}
