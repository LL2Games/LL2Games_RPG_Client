#include "Projectile.h"


#include "stbResourceManager.h"
#include "MonsterDataManager.h"
#include "stbTexture.h"
#include "Util.h"
#include "Collider_Info.h"
#include "stbLogger.h"
#include <algorithm>
#include <cmath>

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

    // Server coordinates share the client world axes/units (x right, y down).
    // Rendering applies camera translation; do not flip or scale the direction.
	m_position = info.pos;
    m_travelled = 0.0f;
	m_direction = { info.dirX, info.dirY };

	if (m_transform != nullptr)
	{
		m_transform->SetPosition(m_position);
	}

	SetAnimation();
	SetCollider();
	
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


void Projectile::Update(float deltaTime)
{
    GameObject::Update();
    const float dt = (std::max)(deltaTime, 0.0f);
    const auto before = m_position;
    const stb::math::Vector2 delta = {m_direction.x * m_speed * dt, m_direction.y * m_speed * dt};
    m_position += delta;
    m_travelled += std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (m_transform) m_transform->SetPosition(m_position);
    M_LOGGER("[Projectile move] instanceId=%d before=(%.6f,%.6f) after=(%.6f,%.6f) dir=(%.6f,%.6f) speed=%.6f range=%.6f travelled=%.6f dt=%.6f",
        m_instanceId, before.x, before.y, m_position.x, m_position.y,
        m_direction.x, m_direction.y, m_speed, m_range, m_travelled, dt);
}

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
        if (info.anim_name != "projectile") continue;
		std::vector<stb::Texture*> frames;
        std::vector<stb::math::Vector2> frameOffsets;

		for (int i = 0; i < info.frame_count; ++i)
		{
			std::wstring key =
				utils::StringToWString(info.path + "/" + info.frame_prefix + std::to_string(i));

			stb::Texture* tex = M_RESOURCEMANAGER->Find<stb::Texture>(key);
			if (tex != nullptr)
            {
                frames.emplace_back(tex);
                // A projectile uses its own image center, not the monster's body/feet origin.
                // Keep the visible center at the same world offset as the collision shape.
                const auto& center = data->projectileData.colliderInfo.offset;
                frameOffsets.emplace_back(center.x - static_cast<float>(tex->GetWidth()) * 0.5f,
                    center.y - static_cast<float>(tex->GetHeight()) * 0.5f);
            }
		}

		m_animator->CreateFrameAnimation(
			utils::StringToWString(info.anim_name),
			frames,
			stb::math::Vector2::Zero,
            frameOffsets,
            static_cast<float>(info.delay_ms) / 1000.0f
		);

		stb::Animator::EventNames eventNames;

		eventNames.startEventName = utils::StringToWString(info.animationEvent.start);
		eventNames.completeEventName = utils::StringToWString(info.animationEvent.complete);
		eventNames.endEventName = utils::StringToWString(info.animationEvent.end);

		m_animator->SetAnimationEventNames(utils::StringToWString(info.anim_name), eventNames);

	}
}

bool Projectile::IsExpired() const
{
    return m_travelled >= m_range;
}

void Projectile::LogRemoval(const char* reason) const
{
    M_LOGGER("[Projectile remove] instanceId=%d reason=%s dir=(%.6f,%.6f) pos=(%.6f,%.6f) speed=%.6f range=%.6f travelled=%.6f",
        m_instanceId, reason, m_direction.x, m_direction.y, m_position.x, m_position.y,
        m_speed, m_range, m_travelled);
}

void Projectile::SetCollider()
{
	// ownerId가 몬스터 타입 ID라는 현재 전제
	const MonsterData* monsterData =
		M_MONSTERDATAMANAGER->FindMonsterData(m_ownerId);

	if (monsterData == nullptr)
		return;

	const ColliderInfo& info = monsterData->projectileData.colliderInfo;

	switch (info.colliderType)
	{
	case stb::eColliderType::Rect2D:
		m_collider = AddComponent<stb::BoxCollider2D>();
		m_collider->SetSize(info.halfSize);
		break;

	case stb::eColliderType::Circle2D:
		m_collider = AddComponent<stb::CircleCollider2D>();

		// 기존 Collider가 Vector2 size만 지원하므로 임시 호환
		m_collider->SetSize({ info.radius, info.radius });
		break;

	default:
		return;
	}

	m_collider->SetColliderType(info.colliderType);
	m_collider->SetOffset(info.offset);
}
