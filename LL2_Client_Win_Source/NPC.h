#pragma once

#include "stbGameObject.h"
#include "NPC_info.h"

namespace stb
{
    class Transform;
    class Animator;

    class NPC : public GameObject
    {
    public:
        void Initialize() override;
        bool Setup(int spawnId, int npcId);

        ~NPC() override;
    private:
        Transform* m_transform = nullptr;
        Animator* m_animator = nullptr;

        NPCData m_data{};
        int m_spawnId = 0;
    };
}

