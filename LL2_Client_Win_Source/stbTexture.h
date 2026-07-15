#pragma once
#include "stbResource.h"

class stbD2DRenderer;
struct ID2D1Bitmap;

namespace stb
{
	class Texture : public Resource
	{
	public:
		enum class eTextureType
		{
			Bmp,
			Png,
			None,
		};

		static Texture* Create(const std::wstring& name, UINT width, UINT height);

		Texture();
		~Texture();
		
		virtual HRESULT Load(const std::wstring& path) override;
		void LoadD2D(stbD2DRenderer& renderer);

		void SetWidth(UINT width) { mWidth = width;}
		UINT GetWidth() { return mWidth; }
		void SetHegith(UINT height) { mHeight = height; }
		UINT GetHeight() { return mHeight; }
		eTextureType GetTextureType() { return mType; }
		Gdiplus::Image* GetImage() { return mImage; }
		bool IsAlpha() { return mAlpha; }
		HDC GetHdc() { return mHdc; }
		ID2D1Bitmap* GetD2DBitmap() { return mD2DBitmap; }

	private:
		bool mAlpha;
		eTextureType mType;

		HBITMAP mBitmap;
		HDC mHdc;

		UINT mWidth;
		UINT mHeight;

		Gdiplus::Image* mImage;
		ID2D1Bitmap* mD2DBitmap = nullptr;
	};
}

