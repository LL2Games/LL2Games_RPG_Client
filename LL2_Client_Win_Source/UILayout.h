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
    int x;
    int y;
    int width;
    int height;
};

class UILayout
{
public:
    static UIRect CalcRect(
        int windowWidth,
        int windowHeight,
        int uiWidth,
        int uiHeight,
        UIAnchor anchor,
        int offsetX,
        int offsetY);

    static D2D1_RECT_F ToD2DRect(const UIRect& rc);
};
