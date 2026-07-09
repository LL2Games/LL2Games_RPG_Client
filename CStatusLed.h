#pragma once

enum class ConnectionState
{
    DISCONNECTED = 0,
    CONNECTING,
    CONNECTED
};

// CStatusLed

class CStatusLed : public CStatic
{
	DECLARE_DYNAMIC(CStatusLed)

public:
    COLORREF m_color = RGB(255, 0, 0); // ±âº» »¡°­

    void SetColor(COLORREF color)
    {
        m_color = color;
        Invalidate();
    }

protected:
    afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    DECLARE_MESSAGE_MAP()
};


