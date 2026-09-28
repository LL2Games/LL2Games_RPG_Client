#include "stbCamera.h"
#include "stbGameObject.h"
#include "stbTransform.h"
#include "stbApplication.h"
#include <algorithm>

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
		const auto size = M_APP->GetRenderer().GetRenderTargetSize();
    	mResolution = Vector2(size.width, size.height);
	}

	void Camera::Update()
	{
		const auto size = M_APP->GetRenderer().GetRenderTargetSize();

		if (size.width <= 0.0f || size.height <= 0.0f)
		    return;
			
		mResolution = Vector2(size.width, size.height);

        if (mTarget != nullptr)
        {
            Transform* transform = mTarget->GetComponent<Transform>();
            mLookPosition = transform->GetPosition();
        }
        else
        {
            Transform* cameraTransform = GetOwner()->GetComponent<Transform>();

            mLookPosition = cameraTransform->GetPosition();
        }

        Vector2 desiredDistance = (mLookPosition + mLookOffset) - (mResolution / 2.0f);

        float maxX = std::max(0.0f, mWorldSize.x - mResolution.x);
        float maxY = std::max(0.0f, mWorldSize.y - mResolution.y);
        mDistance.x = std::max(0.0f, std::min(desiredDistance.x, maxX));
        mDistance.y = std::max(0.0f,std::min(desiredDistance.y, maxY));
	}

	void Camera::LateUpdate()
	{

	}

	void Camera::Render(HDC /*hdc*/)
	{

	}

}


