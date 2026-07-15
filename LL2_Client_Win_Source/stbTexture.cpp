#include "stbD2DRenderer.h"
#include "stbTexture.h"
#include "stbResourceManager.h"
#include "stbApplication.h"

#define M_RESOURCE stb::SingletonBase<stb::ResourceManager>::getInstance()
#define M_APP stb::SingletonBase<stb::Application>::getInstance()

namespace stb
{
	Texture* Texture::Create(const std::wstring& name, UINT width, UINT height)
	{
		Texture* image = M_RESOURCE->Find<Texture>(name);
		if (image)
		{
			return image;
		}
		else
		{
			image = new Texture();
			image->SetName(name);
			image->SetWidth(width);
			image->SetHegith(height);

			HDC hdc = M_APP->GetHDC();
			HWND hwnd = M_APP->GetHWND();

			image->mBitmap = CreateCompatibleBitmap(hdc, width, height);

			image->mHdc = CreateCompatibleDC(hdc);

			HBRUSH transparentBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, transparentBrush);

			Rectangle(image->mHdc, -1, -1, image->GetWidth() + 1, image->GetHeight() + 1);
			SelectObject(hdc, oldBrush);


			HBITMAP oldBitmap = (HBITMAP)SelectObject(image->mHdc, image->mBitmap);
			DeleteObject(oldBitmap);

			M_RESOURCE->Insert(name, image);
			return image;

		}

	}

	Texture::Texture()
		:Resource(enums::eResourceType::Texture)
		, mAlpha(false)
	{

	}

	Texture::~Texture()
	{
	}

	/*
	LoadImageW(
    _In_opt_ HINSTANCE hInst,
    _In_ LPCWSTR name,
    _In_ UINT type,
    _In_ int cx,
    _In_ int cy,
    _In_ UINT fuLoad);
	*/
	HRESULT Texture::Load(const std::wstring& path)
	{
		std::wstring ext = path.substr(path.find_last_of(L".") + 1);

		if (ext == L"bmp")
		{
			mType = eTextureType::Bmp;
			mBitmap = (HBITMAP)LoadImageW(nullptr, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

			if (mBitmap == nullptr)
			{
				return S_FALSE;
			}

			/*
			 LONG        bmType;
			 LONG        bmWidth;
			 LONG        bmHeight;
			 LONG        bmWidthBytes;
			 WORD        bmPlanes;
			 WORD        bmBitsPixel;
			 LPVOID      bmBits;

			*/

			BITMAP info = {}; 
			// GetObject() = ????? ??? / ?????? ?? / ?????? ????
			GetObject(mBitmap, sizeof(mBitmap), &info);

			mWidth = info.bmWidth;
			mHeight = info.bmHeight;

			if (info.bmBitsPixel == 32)
			{
				mAlpha = true;
			}
			else if (info.bmBitsPixel = 24)
			{
				mAlpha = false;
			}

			HDC mainDC = M_APP->GetHDC();
			mHdc = CreateCompatibleDC(mainDC);


			HBITMAP oldBitmap = (HBITMAP)SelectObject(mHdc, mBitmap);
			DeleteObject(oldBitmap);


		}
		else if(ext == L"png")
		{
			mType = eTextureType::Png;
			mImage = Gdiplus::Image::FromFile(path.c_str());

			// nullptr 체크 + GDI+ 상태 체크 (FromFile은 실패해도 nullptr 반환 안 함)
			if (mImage == nullptr || mImage->GetLastStatus() != Gdiplus::Ok)
			{
				// 어떤 경로를 찾고 있는지 출력
				wchar_t debugMsg[512];
				swprintf_s(debugMsg, L"[Texture] PNG 로드 실패: %s\n", path.c_str());
				OutputDebugStringW(debugMsg);

				// 현재 작업 디렉토리 출력
				wchar_t cwd[512];
				GetCurrentDirectoryW(512, cwd);
				wchar_t cwdMsg[512];
				swprintf_s(cwdMsg, L"[Texture] 현재 작업 디렉토리: %s\n", cwd);
				OutputDebugStringW(cwdMsg);

				delete mImage;
				mImage = nullptr;
				return S_FALSE;
			}

			mWidth = mImage->GetWidth();
			mHeight = mImage->GetHeight();		
		}

		return S_OK;
	}

	/*
		이미 읽어온 이미지들을 화면에 그릴 수 있게 미리 ID2D1Bitmap을 만들어두는 것
	*/
	void Texture::LoadD2D(stbD2DRenderer& renderer)
	{
		if (mD2DBitmap)
			return;

		HRESULT hr = renderer.CreateBitmapFromFile(GetPath().c_str(), &mD2DBitmap);
		if (FAILED(hr))
		{
			wchar_t msg[512];
			swprintf_s(msg, L"[Texture] D2D 비트맵 로드 실패: %s\n", GetPath().c_str());
			OutputDebugStringW(msg);
		}
	}

}