#pragma once
#include "CommonInclude.h"
#include <d2d1.h>

enum class UIAnchor
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Center,
    CenterBottom,

};

struct UIRect
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

class UILayout
{
public:
    static UIRect CalcRect(
        float windowWidth,
        float windowHeight,
        float uiWidth,
        float uiHeight,
        UIAnchor anchor,
        float offsetX,
        float offsetY);

    static D2D1_RECT_F ToD2DRect(const UIRect& rc);
};
