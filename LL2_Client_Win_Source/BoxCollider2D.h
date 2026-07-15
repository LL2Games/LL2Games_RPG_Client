#pragma once
#include "stbCollider.h"



namespace stb
{
	class BoxCollider2D : public Collider
	{
	public:
		BoxCollider2D();
		~BoxCollider2D();

	    void Initialize() override;
        void Update() override;
        void LateUpdate() override;
        void Render(HDC hdc) override;
		void Render(stbD2DRenderer& renderer) override;

	public:
	
	private:
	};

}

