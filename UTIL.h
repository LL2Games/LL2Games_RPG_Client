#pragma once
#include <Windows.h>
#include <vector>
#include <string>

class UTIL
{
public:
    static uint16_t ParseHexType(const CString& str);
    static std::vector<std::string> ParsePayload(const CString& str);
    static CString Utf8ToCString(const std::string& utf8);
};

