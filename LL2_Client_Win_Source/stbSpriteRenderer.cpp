#include "stbSpriteRenderer.h"
#include "stbTransform.h"
#include "stbGameObject.h"
#include "stbTexture.h"
#include "stbSpriteRenderer.h"

namespace stb
{
	SpriteRenderer::SpriteRenderer()
		: Component(enums::eComponentType::SpriteRenderer)
		, mTexture(nullptr)
		, mSize(Vector2::One)
	{

	}
	SpriteRenderer::~SpriteRenderer()
	{
	}
	void SpriteRenderer::Initialize()
	{
	}
	void SpriteRenderer::Update()
	{
	}
	void SpriteRenderer::LateUpdate()
	{
	}
	void SpriteRenderer::Render(HDC hdc)
	{

		if (mTexture == nullptr)
		{
			assert(false);
		}

		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();
		float rot = tr->GetRotation();
		Vector2 scale = tr->GetScale();

		if (mTexture->GetTextureType() == Texture::eTextureType::Bmp)
		{
			if (mTexture->IsAlpha())
			{
				/*
				   BLENDFUNCTION 구조체는 원본 및 대상 비트맵에 대한 혼합 함수를 지정하여 혼합을 제어합니다.
				   BYTE   BlendOp;	// AC_SRC_OVER
				   BYTE   BlendFlags; // 0
				   BYTE   SourceConstantAlpha; // 투명도 값
				   BYTE   AlphaFormat; //AC_SRC_ALPHA
								
				*/
				BLENDFUNCTION func = {};

				func.BlendOp = AC_SRC_OVER;
				func.BlendFlags = 0;
				func.SourceConstantAlpha = 255;
				func.AlphaFormat = AC_SRC_ALPHA;
				/*
				 _In_ HDC hdcDest,
				 _In_ int xoriginDest,
				 _In_ int yoriginDest,
				 _In_ int wDest,
				 _In_ int hDest,
				 _In_ HDC hdcSrc,
				 _In_ int xoriginSrc,
				 _In_ int yoriginSrc,
				 _In_ int wSrc,
				 _In_ int hSrc,
				 _In_ BLENDFUNCTION ftn
								*/
				AlphaBlend(hdc
					, pos.x, pos.y
					, mTexture->GetWidth()  * mSize.x * scale.x
					, mTexture->GetHeight() * mSize.y * scale.y
					, mTexture->GetHdc()
					, 0, 0
					, mTexture->GetWidth()
					, mTexture->GetHeight()
					, func);
			}
			else
			{
				TransparentBlt(hdc
					, pos.x, pos.y
					, mTexture->GetWidth() *  mSize.x * scale.x
				    , mTexture->GetHeight() * mSize.y * scale.y
					, mTexture->GetHdc()
					, 0,0
					, mTexture->GetWidth()
				    , mTexture->GetHeight()
					, RGB(255,0,255));
			}

			/*
			  _In_ HDC hdcDest, // 대상 디바이스
			  _In_ int xoriginDest, 대상 디바이스 x
			  _In_ int yoriginDest, 대상 디바이스 y
			  _In_ int wDest,		대상 Width
			  _In_ int hDest,		대상 Height
			  _In_ HDC hdcSrc,		원본 디바이스
			  _In_ int xoriginSrc,	원본 x
			  _In_ int yoriginSrc,	원본 y
			  _In_ int wSrc,		원본 Width
			  _In_ int hSrc,		원본 Heigth
			  _In_ UINT crTransparent); 투명처리할 RGB
			
			*/
		
		}
		else if (mTexture->GetTextureType() == Texture::eTextureType::Png)
		{


			// 내가 원하는 픽셀을 투명화 시킬때
			//Gdiplus::ImageAttributes imgAtt = {};
			//
			//// 투명화 시킬 픽셀의 색 범위
			//imgAtt.SetColorKey(Gdiplus::Color(230, 230, 230), Gdiplus::Color(255, 255, 255));


			Gdiplus::Graphics graphics(hdc);

			graphics.TranslateTransform(pos.x, pos.y);
			graphics.RotateTransform(rot);
			graphics.TranslateTransform(-pos.x, -pos.y);

			graphics.DrawImage(mTexture->GetImage() 
				, Gdiplus::Rect(
					pos.x, pos.y
					, mTexture->GetWidth() * mSize.x * scale.x
					, mTexture->GetHeight() * mSize.y * scale.y
				)
				, 0, 0
				, mTexture->GetWidth(), mTexture->GetHeight()
				, Gdiplus::UnitPixel
				, nullptr);
			
		}
	}
}