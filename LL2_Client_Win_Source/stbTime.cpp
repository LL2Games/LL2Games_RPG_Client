#include "stbTime.h"

namespace stb
{
	Time::Time()
		: cpu_frequency({})
		, startTime({})
		, currTime({})
		, deltaTime(0.0f)
	{

	}

	Time::~Time()
	{

	
	}
	void Time::Initialize()
	{
		QueryPerformanceFrequency(&cpu_frequency);
		QueryPerformanceCounter(&startTime);


	}

	void Time::Update()
	{
		QueryPerformanceCounter(&currTime);

		LONGLONG differenceFrequency
			= currTime.QuadPart - startTime.QuadPart;

		deltaTime = static_cast<float>(differenceFrequency) / static_cast<float>(cpu_frequency.QuadPart);

		startTime.QuadPart = currTime.QuadPart;

	}

	void Time::Render(HDC hdc)
	{
		static float time = 0.0f;

		time += deltaTime;

		float fps = 1.0f / deltaTime;

		wchar_t str[50] = L"";
		swprintf_s(str, 50, L"Time : %d", (int)fps);

		int len = wcsnlen_s(str, 50);


		TextOut(hdc, 0, 0, str, len);
	}

	void Time::Render(stbD2DRenderer& renderer)
	{
		static float time = 0.0f;

		time += deltaTime;

		float fps = 1.0f / deltaTime;

		wchar_t str[50] = L"";
		swprintf_s(str, 50, L"Time : %d", (int)fps);

		int len = wcsnlen_s(str, 50);
		
		renderer.DrawTextString(
			str,
			D2D1::RectF(0.0f, 0.0f, 200.0f, 40.0f),
			D2D1::ColorF(D2D1::ColorF::Black));
	}
}