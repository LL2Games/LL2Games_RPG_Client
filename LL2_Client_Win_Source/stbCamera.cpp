#include "stbCamera.h"
#include "stbGameObject.h"
#include "stbTransform.h"
#include "stbApplication.h"

#define M_APP stb::SingletonBase<stb::Application>::getInstance()

namespace stb
{
	Camera::Camera()
		: Component(eComponentType::Camera)
		, mTarget(nullptr)
		, mDistance(Vector2::Zero)
		, mResolution(Vector2::Zero)
		, mLookPosition(Vector2::Zero)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::Initialize()
	{
		mResolution.x = M_APP->GetWidth();
		mResolution.y = M_APP->GetHeight();
	}

	void Camera::Update()
	{
		if (mTarget)
		{
			Transform* tr = mTarget->GetComponent<Transform>();
			mLookPosition = tr->GetPosition();
		}
		else
		{
			Transform* cameraTr = GetOwner()->GetComponent<Transform>();
			mLookPosition = cameraTr->GetPosition();
		}

		// 화면 크기의 중간 위치에서 위치를 빼기

		mDistance = mLookPosition - (mResolution / 2.0f);
	}

	void Camera::LateUpdate()
	{

	}

	void Camera::Render(HDC hdc)
	{

	}

}


