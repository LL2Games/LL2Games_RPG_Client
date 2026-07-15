#pragma once
#include "stbSingletonBase.h"
#include "CommonInclude.h"
#include "stbD2DRenderer.h"

namespace stb
{
	class Time : public SingletonBase<Time>
	{
	public:
		Time();
		~Time();

		void Initialize();
		void Update();
		void Render(HDC hdc);
		void Render(stbD2DRenderer& renderer);

		float GetDeltaTime() { return deltaTime; }

	private:
		// 프로그램 시작 할 때 cpu 진동수 받아오는 변수 하나 / 현재 진동 받는 변수 / 

		LARGE_INTEGER cpu_frequency;
		LARGE_INTEGER startTime;
		LARGE_INTEGER currTime;
		float deltaTime;
		// DeltaTime
		// (startTime - currTime) / cpu 진동수
		// 

	};

}


