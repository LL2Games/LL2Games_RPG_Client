#pragma once
#include "MovementTypes.h"
#include "stbAnimator.h"
#include <algorithm>
#include <cmath>

namespace movement
{
    class Visual
    {
    public:
        void Reset() { previousMode = Mode::Grounded; landingTime = 0; }
        void Update(stb::Animator* animator, const Snapshot& s, float dt, bool ladder, bool action)
        {
            if (!animator) return;
            if ((previousMode == Mode::Rising || previousMode == Mode::Falling) && s.mode == Mode::Grounded)
                landingTime = 0.1f;
            previousMode = s.mode;
            if (s.mode != Mode::Grounded) landingTime = 0;
            const bool monster = s.kind == Kind::Monster;
            animator->SetFlipX(s.facing > 0);
            if (action && !IsDead(s))
            {
                animator->SetPaused(false); landingTime = 0; return;
            }
            std::wstring name;
            bool loop = true;
            if (IsDead(s)) { name = monster ? L"die" : L"dead"; loop = false; }
            else if (IsStunned(s)) { name = L"hit"; loop = false; }
            else if (s.mode == Mode::Rising) name = L"jump";
            else if (s.mode == Mode::Falling) name = L"fall";
            else if (s.mode == Mode::Climbing) name = ladder ? L"ladder" : L"rope";
            else if (landingTime > 0) { name = L"land"; loop = false; }
            else if (std::abs(s.velocity.x) > 0.01f) name = monster ? L"move" : L"walk";
            else name = monster ? L"idle" : L"stand";
            landingTime = (std::max)(0.0f, landingTime - dt);
            if (!animator->FindAnimation(name))
                name = (!monster && s.mode == Mode::Falling && animator->FindAnimation(L"jump"))
                    ? L"jump" : (monster ? L"idle" : L"stand");
            if (animator->FindAnimation(name) && !animator->IsPlaying(name)) animator->PlayAnimation(name, loop);
            animator->SetPaused(!IsDead(s) && s.mode == Mode::Climbing && std::abs(s.velocity.y) < 0.01f);
        }
    private:
        Mode previousMode = Mode::Grounded;
        float landingTime = 0;
    };
}
