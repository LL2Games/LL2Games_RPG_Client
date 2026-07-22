#include "stbCollider.h"

unsigned int stb::Collider::CollisionID = 0;

namespace stb
{
	Collider::Collider()
		: Component(eComponentType::Collider)
	{
	}

	Collider::~Collider()
	{
	}


	void Collider::Initialize()
	{	 
	}	 
		 
	void Collider::Update()
	{	 
	}	 
		 
	void Collider::LateUpdate()
	{	 
	}	 
		 
	void Collider::Render(HDC /*hdc*/)
	{	 
	}	 

	void Collider::Render(stbD2DRenderer& /*renderer*/)
	{

	}
		 
	void Collider::OnCollisionEnter(Collider* /*other*/)
	{	 
	}	 
		 
	void Collider::OnCollisionStay(Collider* /*other*/)
	{	 
	}	 
		 
	void Collider::OnCollisionEnd(Collider* /*other*/)
	{
	}
}



