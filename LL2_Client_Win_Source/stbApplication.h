#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"
#include "stbD2DRenderer.h"

namespace stb
{
	class Application : public SingletonBase<Application>
	{
	public:
		Application();
		~Application();

		// 윈도우의 사이즈와 핸들을 받아온다.
		bool Initialize(HWND hWnd, UINT width, UINT height);
		void Run();
		void Update();
		void Render();
		void Destroy();
		void Release();

		HWND GetHWND() { return mHwnd; }
		HDC GetHDC() { return mHdc; }
		UINT GetWidth() { return mWidth; }
		UINT GetHeight() { return mHeight; }
		stbD2DRenderer& GetRenderer() { return m_Renderer; }

	private:
		void ClearRenderTarget();
		void adjustWindowRect(HWND hWnd, UINT width, UINT height);
		void createBuffer(UINT width, UINT height);
		void copyDC(HDC source, HDC dest);
	

	private:
		HWND mHwnd;
		HDC  mHdc;
		HDC  mBackHDC;
		HBITMAP mBackBuffer;
		UINT mWidth;
		UINT mHeight;
		
		stbD2DRenderer m_Renderer;



	};
}

