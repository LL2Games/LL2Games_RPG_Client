#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <dwrite.h>
#include <string>



#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;

class D2DRenderer
{
public:
	bool Initialize(HWND hwnd);
	void ShutDown();
	void Resize(UINT width, UINT height);

	bool LoadBitmap(const std::wstring& filePath);


	void BeginFrame();
	void EndFrame();
	void Clear(const D2D1::ColorF& color);

	void DrawLine(float x1, float y1, float x2, float y2, const D2D1::ColorF& color, float stroke = 1.0f);
	void DrawRect(float x, float y, float w, float h, const D2D1::ColorF& color, float stroke = 1.0f);
	void FillRect(float x, float y, float w, float h, const D2D1::ColorF& color);

	void DrawCircle(float cx, float cy, float radius, const D2D1::ColorF& color, float stroke = 1.0f);
	void FillCircle(float cx, float cy, float radius, const D2D1::ColorF& color);

	void DrawTextString(const std::wstring& text, const D2D1_RECT_F& layoutRect, const D2D1::ColorF& color, bool titleStyle = false);
	void DrawBitmap(float x, float y, float width = -1.0f, float height = -1.0f, float opacity = 1.0f);

	bool HasBitmap() const;


private:
	bool CreateTextFormats();
	bool CreateDeviceResources();
	void DiscardDeviceResources();
	bool EnsureBrush();

	HRESULT CreateBitmapFromFile(PCWSTR filePath, ID2D1Bitmap** outBitmap);
	
private:
	HWND mHwnd = nullptr;

	ComPtr<ID2D1Factory> m_D2DFactory;
	ComPtr<ID2D1HwndRenderTarget> m_RenderTarget;
	ComPtr<ID2D1SolidColorBrush> m_Brush;

	ComPtr<IDWriteFactory> m_DWriteFactory;
	ComPtr<IDWriteTextFormat> m_TitleTextFormat;
	ComPtr<IDWriteTextFormat> m_BodyTextFormat;
	
	// WIC에서 디코더, 포멧 컨버터 같은 걸 만드는 팩토리
	ComPtr<IWICImagingFactory> m_WicFactory;
	ComPtr<ID2D1Bitmap> m_Bitmap;

	std::wstring m_BitmapFilePath;
};

