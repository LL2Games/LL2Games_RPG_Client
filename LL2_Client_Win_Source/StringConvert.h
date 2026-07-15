#pragma once
#include <charconv>
#include <string_view>
#include <system_error>
#include <string>
#include <Windows.h>


namespace Convert
{
	/*
	string_view??臾몄옄?댁쓣 蹂듭궗?섏? ?딄퀬 李몄“留??섎뒗 媛앹껜?대떎.
	*/
	inline bool StringToInt(std::string_view s, int& output)
	{
		// string??data ?⑥닔??臾몄옄?댁쓽 ?쒖옉 二쇱냼瑜?媛?몄삩??
		const char* b = s.data();
		const char* e = s.data() + s.size();

		// from_chars??b遺??e?꾧퉴吏 臾몄옄瑜??쎌뼱???뺤닔濡??댁꽍?섍퀬 洹?寃곌낵瑜?output????ν븳??
		// 援ъ“ 遺꾪빐 臾몃쾿?쇰줈??p : ?대뵒源뚯? ?쎌뿀?붿? 媛由ы궎???ъ씤?? ec : ?먮윭 肄붾뱶
		auto [p, ec] = std::from_chars(b, e, output);
		// ec == std::errc{}???섎????먮윭 ?놁쓬
		return ec == std::errc{} && p == e;
	}

	inline bool StringToInt64(std::string_view s, int64_t& output)
	{
		// string??data ?⑥닔??臾몄옄?댁쓽 ?쒖옉 二쇱냼瑜?媛?몄삩??
		const char* b = s.data();
		const char* e = s.data() + s.size();

		// from_chars??b遺??e?꾧퉴吏 臾몄옄瑜??쎌뼱???뺤닔濡??댁꽍?섍퀬 洹?寃곌낵瑜?output????ν븳??
		// 援ъ“ 遺꾪빐 臾몃쾿?쇰줈??p : ?대뵒源뚯? ?쎌뿀?붿? 媛由ы궎???ъ씤?? ec : ?먮윭 肄붾뱶
		auto [p, ec] = std::from_chars(b, e, output);
		// ec == std::errc{}???섎????먮윭 ?놁쓬
		return ec == std::errc{} && p == e;
	}

	inline bool StringToFloat(const std::string& s, float& out)
	{
		char* end = nullptr;
		errno = 0;
		out = std::strtof(s.c_str(), &end);
		if (errno != 0) return false;                // 踰붿쐞 ?ㅻ쪟 ??
		if (end == s.c_str()) return false;          // 蹂?섎맂 寃??놁쓬
		return *end == '\0';                         // ?앷퉴吏 ???뚮퉬?덈뒗吏
	};

	// wstring(UTF-16) ??string(UTF-8) 蹂??
	inline std::string WstrToUtf8(const std::wstring& wstr)
	{
		if (wstr.empty())
			return {};

		int utf8Len = WideCharToMultiByte(
			CP_UTF8, 0,
			wstr.c_str(), (int)wstr.size(),
			nullptr, 0,
			nullptr, nullptr);

		if (utf8Len <= 0)
			return {};

		std::string result(utf8Len, '\0');
		WideCharToMultiByte(
			CP_UTF8, 0,
			wstr.c_str(), (int)wstr.size(),
			&result[0], utf8Len,
			nullptr, nullptr);

		return result;
	}

	// string(UTF-8) ??wstring(UTF-16) 蹂??(??갑?? ?꾩슂 ???ъ슜)
	inline std::wstring Utf8ToWstr(const std::string& str)
	{
		if (str.empty())
			return {};

		int wideLen = MultiByteToWideChar(
			CP_UTF8, 0,
			str.c_str(), (int)str.size(),
			nullptr, 0);

		if (wideLen <= 0)
			return {};

		std::wstring result(wideLen, L'\0');
		MultiByteToWideChar(
			CP_UTF8, 0,
			str.c_str(), (int)str.size(),
			&result[0], wideLen);

		return result;
	}


	inline std::string Utf8ToAnsi(const std::string& utf8)
	{
		if (utf8.empty())
			return std::string();

		// 1. UTF-8 -> UTF-16
		int wideLen = MultiByteToWideChar(
			CP_UTF8,
			0,
			utf8.c_str(),
			-1,
			nullptr,
			0
		);

		if (wideLen <= 0)
			return std::string();

		std::wstring wide;
		wide.resize(wideLen);

		int wideResult = MultiByteToWideChar(
			CP_UTF8,
			0,
			utf8.c_str(),
			-1,
			&wide[0],
			wideLen
		);

		if (wideResult <= 0)
			return std::string();

		// 2. UTF-16 -> CP949
		int ansiLen = WideCharToMultiByte(
			949,    // CP949
			0,
			wide.c_str(),
			-1,
			nullptr,
			0,
			nullptr,
			nullptr
		);

		if (ansiLen <= 0)
			return std::string();

		std::string ansi;
		ansi.resize(ansiLen);

		int ansiResult = WideCharToMultiByte(
			949,    // CP949
			0,
			wide.c_str(),
			-1,
			&ansi[0],
			ansiLen,
			nullptr,
			nullptr
		);

		if (ansiResult <= 0)
			return std::string();

		// resize에 null 문자까지 포함되어 있으므로 제거
		if (!ansi.empty() && ansi.back() == '\0')
			ansi.pop_back();

		return ansi;
	}

	inline std::wstring StringToWString(const std::string& str)
	{
		if (str.empty())
			return L"";

		int sizeNeeded = MultiByteToWideChar(
			CP_UTF8,
			0,
			str.c_str(),
			-1,
			nullptr,
			0
		);

		if (sizeNeeded <= 0)
			return L"";

		std::wstring result(sizeNeeded - 1, L'\0');

		MultiByteToWideChar(
			CP_UTF8,
			0,
			str.c_str(),
			-1,
			result.data(),
			sizeNeeded
		);

		return result;
	}
};

