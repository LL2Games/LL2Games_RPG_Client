#include "stbCircleCollider2D.h"
#include "stbTransform.h"
#include "stbrender.h"
#include "stbGameObject.h"
#include "stbD2DRenderer.h"
namespace stb
{
	CircleCollider2D::CircleCollider2D()
	{

	}


	CircleCollider2D::~CircleCollider2D()
	{

	}


	void CircleCollider2D::Initialize()
	{

	}

	void CircleCollider2D::Update()
	{

	}

	void CircleCollider2D::LateUpdate()
	{

	}

	void CircleCollider2D::Render(HDC hdc)
	{
		Transform* tr = GetOwner()->GetComponent<Transform>();
		Vector2 pos = tr->GetPosition();

		if (render::mainCamera)
		{
			pos = render::mainCamera->CalculatePosition(pos);
		}

		Vector2 offset = GetOffset();

		HBRUSH transparentBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, transparentBrush);

		HPEN transparentPen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
		HPEN oldPen = (HPEN)SelectObject(hdc, transparentPen);

		Vector2 rightBottom;
		rightBottom.x = pos.x + offset.x + 100 * GetSize().x;
		rightBottom.y = pos.y + offset.y + 100 * GetSize().y;

		Ellipse(
			hdc,
			static_cast<int>(pos.x + offset.x),
			static_cast<int>(pos.y + offset.y),
			static_cast<int>(rightBottom.x),
			static_cast<int>(rightBottom.y));

		SelectObject(hdc,oldBrush);
		SelectObject(hdc, oldPen);
		DeleteObject(transparentPen);
	}

	void CircleCollider2D::Render(stbD2DRenderer& renderer)
	{
#ifdef __DEV_COLLIDER
		Transform* tr = GetOwner()->GetComponent<Transform>();
		if (tr == nullptr)
			return;

		Vector2 worldPos = tr->GetPosition();

		// 콜라이더 중심 월드 좌표
		Vector2 colliderCenter = worldPos + GetOffset();

		// 카메라 보정
		Vector2 screenCenter = colliderCenter;
		if (render::mainCamera)
		{
			screenCenter = render::mainCamera->CalculatePosition(colliderCenter);
		}

		Vector2 halfSize = GetSize();

		float left = screenCenter.x - halfSize.x;
		float top = screenCenter.y - halfSize.y;
		float width = halfSize.x * 2.0f;
		float height = halfSize.y * 2.0f;

		renderer.DrawRect(left, top, width, height, D2D1::ColorF::Black);
#else
		(void)renderer;
#endif
	}

}

