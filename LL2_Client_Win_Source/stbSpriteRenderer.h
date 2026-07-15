#pragma once
#include "stbEntity.h"
#include "stbComponent.h"
#include "stbTexture.h"

namespace stb
{
	class SpriteRenderer : public Component
	{
	public:
		SpriteRenderer();
		~SpriteRenderer();

		 void Initialize() override;
		 void Update() override;
		 void LateUpdate() override;
		 void Render(HDC hdc) override;

		 void SetTexture(Texture* texture) { mTexture = texture; }
		 void SetSize(math::Vector2 size) { mSize = size; }
	private:
		Texture* mTexture;
		math::Vector2 mSize;
	};
}

