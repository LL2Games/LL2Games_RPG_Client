#include "stbDamageText.h"
#include "stbGameObject.h"
#include "stbTransform.h"
#include "stbTime.h"
#include "stbRender.h"
#include "stbCamera.h"
#include "stbD2DRenderer.h"

#define M_TIME stb::SingletonBase<stb::Time>::getInstance()

namespace stb
{
    DamageText::DamageText()
        : Component(enums::eComponentType::DamageText)
    {
    }

    void DamageText::AddDamage(int damage)
    {
        DamageTextInfo info;
        info.damage = damage;
        info.offset = math::Vector2(30.0f, -30.0f); //데미지 텍스트 초기 위치
        info.lifeTime = 0.0f;
        info.maxLifeTime = 0.8f;

        m_texts.push_back(info);
    }

    void DamageText::Update()
    {
        float dt = M_TIME->GetDeltaTime();

        for (auto& text : m_texts)
        {
            text.lifeTime += dt;
            text.offset.y -= 40.0f * dt;
        }

        m_texts.erase(
            std::remove_if(m_texts.begin(), m_texts.end(),
                [](const DamageTextInfo& text)
                {
                    return text.lifeTime >= text.maxLifeTime;
                }),
            m_texts.end()
        );
    }

    void DamageText::Render(stbD2DRenderer& renderer)
    {
        GameObject* owner = GetOwner();
        if (owner == nullptr)
            return;

        Transform* tr = owner->GetComponent<Transform>();
        if (tr == nullptr)
            return;

        math::Vector2 basePos = tr->GetPosition();

        if (render::mainCamera)
            basePos = render::mainCamera->CalculatePosition(basePos);

        for (const auto& text : m_texts)
        {
            float alpha = 1.0f - (text.lifeTime / text.maxLifeTime);
            math::Vector2 pos = basePos + text.offset;

            std::wstring damageText = std::to_wstring(text.damage);

            D2D1_RECT_F rect = D2D1::RectF(
                pos.x - 50.0f,
                pos.y - 20.0f,
                pos.x + 50.0f,
                pos.y + 20.0f
            );

            renderer.DrawTextString(
                damageText,
                rect,
                D2D1::ColorF(0.0f, 0.35f, 1.0f, alpha), //진한파랑
                //D2D1::ColorF(1.0f, 0.1f, 0.1f, alpha), //빨강
                TextStyle::Title
            );
        }
    }
}