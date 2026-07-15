#include "BoxCollider2D.h"
#include "stbTransform.h"
#include "stbGameObject.h"
#include "stbRender.h"
#include "stbD2DRenderer.h"
namespace stb
{
	BoxCollider2D::BoxCollider2D()
	{

	}

	BoxCollider2D::~BoxCollider2D()
	{

	}

	void BoxCollider2D::Initialize()
	{

	}

	void BoxCollider2D::Update()
	{

	}

	void BoxCollider2D::LateUpdate()
	{

	}

	void BoxCollider2D::Render(HDC hdc)
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

		Rectangle(hdc
			, pos.x + offset.x
			, pos.y + offset.y
			, rightBottom.x
			, rightBottom.y);

		SelectObject(hdc, oldBrush);
		SelectObject(hdc, oldPen);
		DeleteObject(transparentPen);
		
	}

	void BoxCollider2D::Render(stbD2DRenderer& renderer)
	{
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

		//renderer.DrawRect(left, top, width, height, D2D1::ColorF::Black);
	}
}

