#pragma once
#include "stbScript.h"
#include "stbMath.h"

#include <functional>

namespace stb
{
    class NPCInteraction : public Script
    {
    public:
        // 서버 전송에 성공적으로 접수되면 true 반환
        using RequestCallback = std::function<bool(int spawnId)>;

        void Setup(int spawnId, int npcId, float interactionRange = 100.0f);

        void SetRequestCallback(RequestCallback callback);

        // JSON의 render 크기와 origin을 전달
        void SetRenderInfo(math::Vector2 size, math::Vector2 origin);

        // 마우스 좌표는 ScreenToClient를 적용한 좌표
        bool HitTest(float mouseX, float mouseY);
        bool IsInRange();

        // NPCManager가 선택한 NPC 하나에만 호출
        bool TryInteract();

        // 성공/실패 응답, 타임아웃, 맵 이탈 시 호출
        void ResetRequest() { m_requestPending = false; }

        int GetSpawnId() const { return m_spawnId; }
        int GetNpcId() const { return m_npcId; }

    private:
        int m_spawnId = 0;
        int m_npcId = 0;

        float m_interactionRange = 100.0f;
        bool m_requestPending = false;

        math::Vector2 m_renderSize = { 96.0f, 96.0f };
        math::Vector2 m_origin = { 48.0f, 92.0f };

        RequestCallback m_onRequest;
    };
}


