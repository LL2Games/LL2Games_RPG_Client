#pragma once
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <dwrite.h>
#include <string>



#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

using Microsoft::WRL::ComPtr;


enum class TextStyle
{
	Body,
	Title,
	Small,
	QuickSlot,
	NickName,
	Trade,
	TradeButton,
	Chat,
	EXP,
};

class stbD2DRenderer
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

	void DrawTextString(const std::wstring& text, const D2D1_RECT_F& layoutRect, const D2D1::ColorF& color, TextStyle TextStyle = TextStyle::Title);
	void DrawBitmap(float x, float y, float width = -1.0f, float height = -1.0f, float opacity = 1.0f);
	void DrawBitmap(ID2D1Bitmap* bitmap, float x, float y, float width = -1.0f, float height = -1.0f, float opacity = 1.0f);
	void DrawBitmap(ID2D1Bitmap* bitmap, const D2D1_RECT_F& destRect, const D2D1_RECT_F& srcRect, float opacity);
	void DrawBitmap(ID2D1Bitmap* bitmap, const D2D1_RECT_F& destRect, const D2D1_RECT_F& srcRect, float opacity, bool flipX);
	void DrawSprite(ID2D1Bitmap* bitmap, float destX, float destY, float destW, float destH, float srcX, float srcY, float srcW, float srcH, float opacity = 1.0f);
	void DrawSprite(ID2D1Bitmap* bitmap, float destX, float destY, float destW, float destH, float srcX, float srcY, float srcW, float srcH, bool flipX, float opacity = 1.0f);

	void DrawSprite2(ID2D1Bitmap* bitmap, float destX, float destY, float destW, float destH, float srcX, float srcY, float srcW, float srcH, bool flipX, float opacity = 1.0f);

	bool HasBitmap() const;
	HRESULT CreateBitmapFromFile(PCWSTR filePath, ID2D1Bitmap** outBitmap);

public:
	D2D1_SIZE_F GetRenderTargetSize() const;
private:
	bool CreateTextFormats();
	bool CreateDeviceResources();
	void DiscardDeviceResources();
	bool EnsureBrush();

public:
	ID2D1HwndRenderTarget* GetRenderTarget() { return m_RenderTarget.Get(); }
	IWICImagingFactory* GetWicFactory() { return m_WicFactory.Get(); }
	IDWriteFactory* GetWriteFactory() { return m_DWriteFactory.Get(); }

private:
	HWND mHwnd = nullptr;

	ComPtr<ID2D1Factory> m_D2DFactory;
	ComPtr<ID2D1HwndRenderTarget> m_RenderTarget;
	ComPtr<ID2D1SolidColorBrush> m_Brush;

	ComPtr<IDWriteFactory> m_DWriteFactory;
	ComPtr<IDWriteTextFormat> m_TitleTextFormat;
	ComPtr<IDWriteTextFormat> m_BodyTextFormat;
	ComPtr<IDWriteTextFormat> m_SmallTextFormat;
	ComPtr<IDWriteTextFormat> m_QuickSlotTextFormat;
	ComPtr<IDWriteTextFormat> m_NicknameTextFormat;
	ComPtr<IDWriteTextFormat> m_ExpTextFormat;
	ComPtr<IDWriteTextFormat> m_TradeTextFormat;
	ComPtr<IDWriteTextFormat> m_TradeButtonTextFormat;
	ComPtr<IDWriteTextFormat> m_ChatTextFormat;
	// WIC에서 디코더, 포멧 컨버터 같은 걸 만드는 팩토리
	ComPtr<IWICImagingFactory> m_WicFactory;
	ComPtr<ID2D1Bitmap> m_Bitmap;

	std::wstring m_BitmapFilePath;
};

