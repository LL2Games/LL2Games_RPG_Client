#pragma once
#include <Windows.h>

// Input polls gameplay keys directly. Do not send Alt combinations to Windows menus.
inline bool ConsumeGameplaySystemKey(UINT message, WPARAM key)
{
    if (message == WM_SYSCHAR || message == WM_SYSDEADCHAR) return true;
    if (message != WM_SYSKEYDOWN && message != WM_SYSKEYUP) return false;
    // Keep OS window switching and closing shortcuts.
    return key != VK_TAB && key != VK_ESCAPE && key != VK_F4;
}
