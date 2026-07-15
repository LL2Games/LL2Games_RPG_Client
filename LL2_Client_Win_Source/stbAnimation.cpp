#include "stbAnimation.h"
#include "stbTime.h"
#include "stbGameObject.h"
#include "stbAnimator.h"
#include "stbTransform.h"
#include "stbRender.h"
#include "stbD2DRenderer.h"
#include "stbLogger.h"

#define M_TIME stb::SingletonBase<stb::Time>::getInstance()

namespace stb
{
  Animation::Animation()
      : Resource(enums::eResourceType::Animation)
    {
    }

   Animation::~Animation()
    {
    }

    void Animation::Update()
    {
        if (mbComplete)
        {
            return;
        }

        mTime += M_TIME->GetDeltaTime();

        if (mAnimationSheet[mIndex].duration < mTime)
        {
            mTime = 0.0f;

            if (mIndex < mAnimationSheet.size() - 1)
            {
                mIndex++;
            }
            else
            {
                mbComplete = true;
            }
        }

    }

    void Animation::Render(HDC hdc)
    {
		OutputDebugStringA("[Animation::Render] 호출됨\n");

		if (mTexture == nullptr)
		{
			OutputDebugStringA("[Animation::Render] mTexture == nullptr, 리턴\n");
			return;
		}
		OutputDebugStringA("[Animation::Render] 텍스처 있음, 그리기 시도\n");

		// 값 확인용 - 한 번만 출력
		static bool once = false;
		if (!once)
		{
			once = true;
			char msg[512];
			GameObject* dbgObj = mAnimator->GetOwner();
			Transform* dbgTr = dbgObj->GetComponent<Transform>();
			Vector2 dbgPos = dbgTr->GetPosition();
			Vector2 dbgScale = dbgTr->GetScale();
			Sprite dbgSprite = mAnimationSheet[mIndex];
			sprintf_s(msg, "[DEBUG] pos=(%.1f,%.1f) scale=(%.1f,%.1f) leftTop=(%.1f,%.1f) size=(%.1f,%.1f) texW=%d texH=%d type=%d\n",
				dbgPos.x, dbgPos.y, dbgScale.x, dbgScale.y,
				dbgSprite.leftTop.x, dbgSprite.leftTop.y,
				dbgSprite.size.x, dbgSprite.size.y,
				mTexture->GetWidth(), mTexture->GetHeight(),
				(int)mTexture->GetTextureType());
			OutputDebugStringA(msg);
		}

		GameObject* gameObj = mAnimator->GetOwner();
		Transform* tr = gameObj->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();
		float rot = tr->GetRotation();
		Vector2 scale = tr->GetScale();

		if (render::mainCamera)
		{
			pos = render::mainCamera->CalculatePosition(pos);
		}

		Sprite sprite = mAnimationSheet[mIndex];
		Texture::eTextureType type = mTexture->GetTextureType();
		if (type == Texture::eTextureType::Bmp)
		{
			HDC imgHDC = mTexture->GetHdc();

			if (mTexture->IsAlpha())
			{
				BLENDFUNCTION func = {};
				func.BlendOp = AC_SRC_OVER;
				func.BlendFlags = 0;
				func.AlphaFormat = AC_SRC_ALPHA;
				func.SourceConstantAlpha = 255;  // 0(Transparent) ~255(Opaque);

				AlphaBlend(hdc
					, pos.x - (sprite.size.x / 2.0f) + sprite.offset.x
					, pos.y - (sprite.size.y / 2.0f) + sprite.offset.y
					, sprite.size.x * scale.x
					, sprite.size.y * scale.y
					, imgHDC
					, sprite.leftTop.x
					, sprite.leftTop.y
					, sprite.size.x
					, sprite.size.y
					, func);
			}
			else
			{
				TransparentBlt(hdc
					, pos.x - (sprite.size.x / 2.0f) + sprite.offset.x
					, pos.y - (sprite.size.y / 2.0f) + sprite.offset.y
					, sprite.size.x * scale.x
					, sprite.size.y * scale.y
					, imgHDC
					, sprite.leftTop.x
					, sprite.leftTop.y
					, sprite.size.x
					, sprite.size.y
					, RGB(255, 0, 255));
			}


			Rectangle(hdc, pos.x, pos.y, pos.x + 10, pos.y + 10);

		}
		else if (type == Texture::eTextureType::Png)
		{
			Gdiplus::Graphics graphics(hdc);

			graphics.TranslateTransform(pos.x, pos.y);
			graphics.RotateTransform(rot);
			graphics.TranslateTransform(-pos.x, -pos.y);

			graphics.DrawImage(mTexture->GetImage()
				, Gdiplus::Rect
				(
					pos.x - (sprite.size.x / 2.0f)
					, pos.y - (sprite.size.y / 2.0f)
					, sprite.size.x * scale.x
					, sprite.size.y * scale.y
				)
				, sprite.leftTop.x
				, sprite.leftTop.y
				, sprite.size.x
				, sprite.size.y
				, Gdiplus::UnitPixel
				, nullptr
			);
		}



    }


	void Animation::Render(stbD2DRenderer& renderer)
	{
		if (mAnimationSheet.empty())
			return;

		GameObject* gameObj = mAnimator->GetOwner();
		Transform* tr = gameObj->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();
		Vector2 scale = tr->GetScale();

		if (render::mainCamera)
			pos = render::mainCamera->CalculatePosition(pos);

		Sprite sprite = mAnimationSheet[mIndex];

		Texture* renderTexture = sprite.texture != nullptr
			? sprite.texture
			: mTexture;

		if (renderTexture == nullptr)
			return;

		ID2D1Bitmap* bitmap = renderTexture->GetD2DBitmap();
		if (bitmap == nullptr)
			return;

		float destX = pos.x - sprite.origin.x + sprite.offset.x;
		float destY = pos.y - sprite.origin.y + sprite.offset.y;
		float destW = sprite.size.x * scale.x;
		float destH = sprite.size.y * scale.y;

	
		renderer.DrawSprite(
			bitmap,
			destX, destY, destW, destH,
			sprite.leftTop.x, sprite.leftTop.y,
			sprite.size.x, sprite.size.y
		);
	}

	void Animation::Render(stbD2DRenderer& renderer, bool flipX)
	{
		if (mAnimationSheet.empty())
			return;

		GameObject* gameObj = mAnimator->GetOwner();
		Transform* tr = gameObj->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();
		Vector2 scale = tr->GetScale();

		if (render::mainCamera)
			pos = render::mainCamera->CalculatePosition(pos);

		Sprite sprite = mAnimationSheet[mIndex];
		//M_LOGGER("mIndex=%d", mIndex);

		Texture* renderTexture = sprite.texture != nullptr
			? sprite.texture
			: mTexture;

		if (renderTexture == nullptr)
			return;

		ID2D1Bitmap* bitmap = renderTexture->GetD2DBitmap();
		if (bitmap == nullptr)
			return;

		float originX = sprite.origin.x;
		float offsetX = sprite.offset.x;

		if (flipX)
		{
			originX = sprite.size.x - sprite.origin.x;
			offsetX = -sprite.offset.x;
		}

		float destX = pos.x - originX + offsetX;
		float destY = pos.y - sprite.origin.y + sprite.offset.y;
		float destW = sprite.size.x * scale.x;
		float destH = sprite.size.y * scale.y;

		renderer.DrawSprite2(
			bitmap,
			destX, destY, destW, destH,
			sprite.leftTop.x, sprite.leftTop.y,
			sprite.size.x, sprite.size.y, flipX);

#if 0 /*플레이어 위치 테스트*/
		// 디버그 1: 실제 Transform 위치
		renderer.DrawRect(
			pos.x - 2.0f,
			pos.y - 2.0f,
			4.0f,
			4.0f,
			D2D1::ColorF::Black
		);

		// 디버그 2: 실제 이미지가 그려지는 박스
		renderer.DrawRect(
			destX,
			destY,
			destW,
			destH,
			D2D1::ColorF::Black
		);
#endif

	}

    void Animation::CreateAnimation(const std::wstring& name
        , Texture* spriteTexture
        , Vector2 leftTop
        , Vector2 size
        , Vector2 offset
        , UINT spriteLength
        , float duration)
    {
        mTexture = spriteTexture;
        for (size_t i = 0; i < spriteLength; i++)
        {
            Sprite sprite = {};
            sprite.leftTop.x = leftTop.x + (size.x * i);
            sprite.leftTop.y = leftTop.y;
            sprite.size = size;
            sprite.offset = offset;
			sprite.origin = Vector2(size.x * 0.5f, size.y * 0.5f);
            sprite.duration = duration;

            mAnimationSheet.push_back(sprite);
        }
    }

	//void Animation::CreateFrameAnimation(const std::wstring& name, const std::vector<Texture*>& frames, Vector2 offset, float duration)
	//{
	//	mTexture = nullptr;

	//	for (Texture* tex : frames)
	//	{
	//		if (tex == nullptr)
	//			continue;

	//		Sprite sprite = {};
	//		sprite.texture = tex;
	//		sprite.leftTop = Vector2(0.0f, 0.0f);
	//		sprite.size = Vector2((float)tex->GetWidth(), (float)tex->GetHeight());
	//		sprite.offset = offset;
	//		sprite.duration = duration;

	//		mAnimationSheet.emplace_back(sprite);
	//	}
	//}

	void Animation::CreateFrameAnimation(const std::wstring& name, const std::vector<Texture*>& frames, Vector2 baseOffset, const std::vector<stb::math::Vector2>& frameOffsets, float duration)
	{
		mTexture = nullptr;
		mAnimationSheet.clear();

		for (size_t i = 0; i < frames.size(); ++i)
		{
			Texture* tex = frames[i];

			if (tex == nullptr)
				continue;

			Vector2 frameOffset = Vector2::Zero;

			if (i < frameOffsets.size())
				frameOffset = frameOffsets[i];

			Sprite sprite = {};
			sprite.texture = tex;
			sprite.leftTop = Vector2(0.0f, 0.0f);
			sprite.size = Vector2((float)tex->GetWidth(), (float)tex->GetHeight());
			sprite.origin = baseOffset;
			sprite.offset = frameOffset;
			sprite.duration = duration;

			mAnimationSheet.emplace_back(sprite);
		}
	}

	void Animation::CreateFrameAnimation(const std::wstring& name, const std::vector<Texture*>& frames, Vector2 origin, Vector2 offset, float duration)
	{
		SetName(name);

		mTexture = nullptr;
		mAnimationSheet.clear();
		mAnimationSheet.reserve(frames.size());

		for (size_t i = 0; i < frames.size(); ++i)
		{
			Texture* tex = frames[i];

			if (tex == nullptr)
				continue;

			const float texWidth = static_cast<float>(tex->GetWidth());
			const float texHeight = static_cast<float>(tex->GetHeight());

			Sprite sprite = {};
			sprite.texture = tex;
			sprite.leftTop = Vector2(0.0f, 0.0f);
			sprite.size = Vector2(texWidth, texHeight);

			// 핵심
			sprite.origin = origin;
			sprite.offset = offset;

			sprite.duration = duration;

			mAnimationSheet.emplace_back(sprite);
		}
	}



    void Animation::Reset()
    {
        mIndex = 0;
        mTime = 0.0f;
        mbComplete = false;
    }

    HRESULT Animation::Load(const std::wstring& path)
    {
        return E_NOTIMPL;
    }

}
