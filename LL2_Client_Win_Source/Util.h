#pragma once
#include <string>

#define TEST_OPACITY 0.8f

namespace utils
{
	inline std::wstring StringToWString(const std::string& str)
	{
		return std::wstring(str.begin(), str.end());
	}
}