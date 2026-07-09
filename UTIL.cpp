#include "pch.h"
#include "UTIL.h"

uint16_t UTIL::ParseHexType(const CString& str)
{
    wchar_t* end = nullptr;
    uint16_t value = static_cast<uint16_t>(
        wcstoul(str, &end, 16)
        );
    return value;
}

std::vector<std::string> UTIL::ParsePayload(const CString& str)
{
    std::vector<std::string> values;

    if (str.IsEmpty())
        return values;

    CString temp = str;
    int pos = 0;

    while (true)
    {
        CString token = temp.Tokenize(L"$", pos);
        if (token.IsEmpty() && pos == -1)
            break;

        values.emplace_back(CT2A(token));
    }

    return values;
}

CString UTIL::Utf8ToCString(const std::string& utf8)
{
    int len = MultiByteToWideChar(
        CP_UTF8, 0,
        utf8.c_str(), (int)utf8.size(),
        nullptr, 0
    );

    CString result;
    wchar_t* buf = result.GetBuffer(len);
    MultiByteToWideChar(
        CP_UTF8, 0,
        utf8.c_str(), (int)utf8.size(),
        buf, len
    );
    result.ReleaseBuffer(len);

    return result;
}