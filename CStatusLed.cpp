// CStatusLed.cpp: 구현 파일
//

#include "pch.h"
#include "LL2Games_Tester.h"
#include "CStatusLed.h"


// CStatusLed

IMPLEMENT_DYNAMIC(CStatusLed, CStatic)

// StatusLed.cpp
BEGIN_MESSAGE_MAP(CStatusLed, CStatic)
END_MESSAGE_MAP()

void CStatusLed::DrawItem(LPDRAWITEMSTRUCT lp)
{
    CDC dc;
    dc.Attach(lp->hDC);

    CRect rect = lp->rcItem;
    dc.FillSolidRect(rect, ::GetSysColor(COLOR_BTNFACE));

    CBrush brush(m_color);
    CBrush* old = dc.SelectObject(&brush);

    dc.Ellipse(rect);

    dc.SelectObject(old);
    dc.Detach();
}
