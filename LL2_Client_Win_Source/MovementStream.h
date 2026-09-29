#pragma once
#include "MovementTypes.h"
#include <algorithm>
#include <limits>

namespace movement
{
    // Server-authoritative rendering. No prediction, replay, or extrapolation.
    class Stream
    {
    public:
        bool Push(const Snapshot& value)
        {
            if (hasSnapshot && (!IsNewer(value, latest) ||
                (waitNewEpoch && value.epoch <= latest.epoch))) return false;
            const bool snap = !hasSnapshot || value.epoch != latest.epoch || IsDead(value) != IsDead(latest);
            latest = value;
            hasSnapshot = true;
            waitNewEpoch = false;
            elapsed = 0;
            if (snap) displayed = value;
            source = displayed;
            return true;
        }
        bool Update(float dt, Snapshot& out)
        {
            if (!Ready()) return false;
            elapsed = (std::min)(elapsed + (std::max)(dt, 0.0f), 0.05f);
            const float alpha = elapsed / 0.05f;
            displayed = latest;
            displayed.position = {
                source.position.x + (latest.position.x - source.position.x) * alpha,
                source.position.y + (latest.position.y - source.position.y) * alpha};
            if (alpha < 1.0f && !IsDead(latest))
            {
                displayed.mode = source.mode;
                displayed.climbableId = source.climbableId;
                displayed.velocity = source.velocity;
            }
            out = displayed;
            return true;
        }
        void Suspend() { waitNewEpoch = true; displayed = source = latest; elapsed = 0; }
        void CancelSuspend() { waitNewEpoch = false; displayed = source = latest; elapsed = 0; }
        void Reset() { *this = Stream{}; }
        bool Ready() const { return hasSnapshot && !waitNewEpoch; }
        bool HasSnapshot() const { return hasSnapshot; }
        const Snapshot& Latest() const { return latest; }
        const Snapshot& Displayed() const { return displayed; }
    private:
        Snapshot latest, source, displayed;
        float elapsed = 0;
        bool hasSnapshot = false;
        bool waitNewEpoch = false;
    };

    class InputSchedule
    {
    public:
        void Reset(int acceptedSequence = 0)
        {
            previous = {}; timer = 0; sent = false; sequence = acceptedSequence;
        }
        bool Poll(const Input& input, float dt, bool force, int& nextSequence)
        {
            timer += (std::max)(dt, 0.0f);
            const bool changed = input.horizontal != previous.horizontal || input.vertical != previous.vertical;
            if (sequence == (std::numeric_limits<int>::max)() ||
                (!force && sent && !changed && !input.jump && timer < 0.1f)) return false;
            previous = input; previous.jump = false;
            timer = 0; sent = true;
            nextSequence = ++sequence;
            return true;
        }
    private:
        Input previous;
        float timer = 0;
        int sequence = 0;
        bool sent = false;
    };
}
