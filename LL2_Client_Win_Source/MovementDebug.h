#pragma once
#include "stbD2DRenderer.h"
#include "stbRender.h"
#include "stbCamera.h"

namespace movement
{
    inline void DrawOriginAndFeet(stbD2DRenderer& renderer, stb::math::Vector2 origin, float footOffset)
    {
#ifdef _DEBUG
        auto feet = origin;
        feet.y += footOffset;
        if (stb::render::mainCamera)
        {
            origin = stb::render::mainCamera->CalculatePosition(origin);
            feet = stb::render::mainCamera->CalculatePosition(feet);
        }
        renderer.DrawLine(origin.x - 5, origin.y, origin.x + 5, origin.y, D2D1::ColorF(D2D1::ColorF::Magenta), 2);
        renderer.DrawLine(origin.x, origin.y - 5, origin.x, origin.y + 5, D2D1::ColorF(D2D1::ColorF::Magenta), 2);
        renderer.DrawLine(feet.x - 7, feet.y, feet.x + 7, feet.y, D2D1::ColorF(D2D1::ColorF::Cyan), 2);
#else
        (void)renderer; (void)origin; (void)footOffset;
#endif
    }
}
