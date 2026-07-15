#include "stbCircleCollider2D.h"
#include "stbTransform.h"
#include "stbrender.h"
#include "stbGameObject.h"
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

		Ellipse(hdc
			, pos.x + offset.x
			, pos.y + offset.y
			, rightBottom.x
			, rightBottom.y);

		SelectObject(hdc,oldBrush);
		SelectObject(hdc, oldPen);
		DeleteObject(transparentPen);
	}

}

