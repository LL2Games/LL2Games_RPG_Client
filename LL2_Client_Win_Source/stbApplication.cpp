#include "stbApplication.h"
#include "stbInput.h"
#include "stbTime.h"
#include "stbSceneManager.h"
#include "stbCollisionManager.h"
#include "ItemDataManager.h"
#include "SkillDataManager.h"
#include "MonsterDataManager.h"



#define M_INPUT stb::SingletonBase<stb::Input>::getInstance()
#define M_TIME stb::SingletonBase<stb::Time>::getInstance()
#define M_SCENEMANAGER stb::SingletonBase<stb::SceneManager>::getInstance()
#define M_COLMANAGER stb::SingletonBase<stb::CollisionManager>::getInstance()
#define M_ITEMDATAMANAGER stb::SingletonBase<ItemDataManager>::getInstance()
#define M_SKILLDATAMANAGER stb::SingletonBase<SkillDataManager>::getInstance()
#define M_MONSTERDATAMANAGER stb::SingletonBase<MonsterDataManager>::getInstance()


namespace stb
{
	Application::Application()
		: mHwnd(nullptr)
		, mWidth(0)
		, mHeight(0)
		//, Player(new GameObject())
		, mHdc(nullptr)
		, mBackHDC(nullptr)
		, mBackBuffer(nullptr)
	{
	}

	Application::~Application()
	{
	}

	bool Application::Initialize(HWND hWnd, UINT width, UINT height)
	{
		bool result = m_Renderer.Initialize(hWnd);

		if (!result)
		{
			MessageBox(hWnd, L"Renderer Initialize Failed", L"Error", MB_OK);
			return false;
		}

		mHwnd = hWnd;
		mWidth = width;
		mHeight = height;

		M_INPUT->Initialize();
		M_TIME->Initialize();
		M_COLMANAGER->Initailzie();
		M_SCENEMANAGER->Initialize();
		
		// Json 파일 실행 시 미리 읽어오기

		if (!M_ITEMDATAMANAGER->Init())
		{
			MessageBox(hWnd, L"fail: M_ITEMDATAMANAGER init", L"Error", MB_ICONERROR);
			return false;
		}

		if (!M_SKILLDATAMANAGER->Init())
		{
			MessageBox(hWnd, L"fail: M_SKILLDATAMANAGER init", L"Error", MB_ICONERROR);
			return false;
		}

		if (!M_MONSTERDATAMANAGER->Init())
		{
			MessageBox(hWnd, L"fail: M_MONSTERDATAMANAGER init", L"Error", MB_ICONERROR);
			return false;
		}

		return true;
	}

	void Application::Run()
	{
		Update();
		Render();
	}

	void Application::Update()
	{
		M_INPUT->Update();
		M_TIME->Update();
		M_COLMANAGER->Update();
		M_SCENEMANAGER->Update();
	}

	void Application::Render()
	{
		m_Renderer.BeginFrame();
		m_Renderer.Clear(D2D1::ColorF(255.f, 255.1f, 255.1f, 255.0f));

		M_TIME->Render(m_Renderer);
		M_SCENEMANAGER->Render(m_Renderer);
		M_COLMANAGER->Render(m_Renderer);


		m_Renderer.EndFrame();
	}

	void Application::Destroy()
	{

	}

	void Application::Release()
	{
	}

	void Application::adjustWindowRect(HWND hWnd, UINT width, UINT height)
	{
		mHwnd = hWnd;
	

		RECT rect = { 0,0, static_cast<LONG>(width), static_cast<LONG>(height)};

		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

		UINT windowWidth = rect.right - rect.left;
		UINT windowHeight = rect.bottom - rect.top;

		SetWindowPos(mHwnd, nullptr, 0, 0, windowWidth, windowHeight, SWP_NOMOVE);

		ShowWindow(mHwnd, true);

		mWidth = width;
		mHeight = height;

	}

	void Application::createBuffer(UINT width, UINT height)
	{
		mBackBuffer = CreateCompatibleBitmap(mHdc, width, height);

		mBackHDC = CreateCompatibleDC(mHdc);

		HBITMAP oldBitmap = (HBITMAP)SelectObject(mBackHDC, mBackBuffer);

		DeleteObject(oldBitmap);
	}

	void stb::Application::copyDC(HDC source, HDC dest)
	{
		BitBlt(dest, 0, 0, mWidth, mHeight, source, 0, 0, SRCCOPY);
	}

	void stb::Application::ClearRenderTarget()
	{

		HBRUSH grayBrush = (HBRUSH)CreateSolidBrush(RGB(255, 255, 255));
		HBRUSH oldBrush = (HBRUSH)SelectObject(mBackHDC, grayBrush);


		Rectangle(mBackHDC, -1, -1, 1001, 1001);

		SelectObject(mBackHDC, oldBrush);
		DeleteObject(grayBrush);

	}
}
