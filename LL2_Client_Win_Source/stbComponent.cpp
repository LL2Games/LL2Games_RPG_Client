#include "stbComponent.h"
#include "stbGameObject.h"
#include "stbD2DRenderer.h"

namespace stb
{
	Component::Component(eComponentType type)
		: mType(type)
	{

	}
	Component::~Component()
	{

	}
	void Component::Initialize()
	{

	}

	void Component::Update()
	{

	}

	void Component::LateUpdate()
	{

	}

	void Component::Render(HDC hdc)
	{

	}

	void Component::Render(stbD2DRenderer& renderer)
	{

	}
}

