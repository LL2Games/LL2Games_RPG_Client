#pragma once
#include "stbComponent.h"
#include "stbmath.h"
#include <vector>
#include <string>

class stbD2DRenderer;

namespace stb
{
    class DamageText : public Component
    {
    public:
        DamageText();

        void AddDamage(int damage);

        void Update() override;
        void Render(stbD2DRenderer& renderer) override;

    private:
        struct DamageTextInfo
        {
            int damage = 0;
            math::Vector2 offset;
            float lifeTime = 0.0f;
            float maxLifeTime = 0.8f;
        };

        std::vector<DamageTextInfo> m_texts;
    };
}