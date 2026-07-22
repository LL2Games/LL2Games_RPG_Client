#define NOMINMAX
#include "UILayout.h"
#include <algorithm>

UIRect UILayout::CalcRect(
    float windowWidth,
    float windowHeight,
    float uiWidth,
    float uiHeight,
    UIAnchor anchor,
    float offsetX,
    float offsetY)
{
    UIRect rc{};
    rc.width = uiWidth;
    rc.height = uiHeight;

    switch (anchor)
    {
    case UIAnchor::TopLeft:
        rc.x = offsetX;
        rc.y = offsetY;
        break;

    case UIAnchor::TopRight:
        rc.x = windowWidth - uiWidth - offsetX;
        rc.y = offsetY;
        break;

    case UIAnchor::BottomLeft:
        rc.x = offsetX;
        rc.y = windowHeight - uiHeight - offsetY;
        break;

    case UIAnchor::BottomRight:
        rc.x = windowWidth - uiWidth - offsetX;
        rc.y = windowHeight - uiHeight - offsetY;
        break;

    case UIAnchor::Center:
        rc.x = (windowWidth - uiWidth) * 0.5f + offsetX;
        rc.y = (windowHeight - uiHeight) * 0.5f + offsetY;
        break;

    case UIAnchor::CenterBottom:
        rc.x = (windowWidth - uiWidth) * 0.5f + offsetX;
        rc.y = windowHeight - uiHeight - offsetY;
        break;
    }

    rc.x = std::max(0.0f, rc.x);
    rc.y = std::max(0.0f, rc.y);

    return rc;
}

 D2D1_RECT_F UILayout::ToD2DRect(const UIRect& rc)
 {
     return D2D1::RectF(
         static_cast<float>(rc.x),
         static_cast<float>(rc.y),
         static_cast<float>(rc.x + rc.width),
         static_cast<float>(rc.y + rc.height)
     );
 }
