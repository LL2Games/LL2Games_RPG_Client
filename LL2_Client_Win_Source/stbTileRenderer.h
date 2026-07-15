#pragma once
#include "stbComponent.h"
#include "stbTexture.h"

namespace stb
{
	class TileRenderer : public Component
	{
	public:
		TileRenderer();
		~TileRenderer();

		 void Initialize() override;
		 void Update() override;
		 void LateUpdate() override;
		 void Render(HDC hdc) override;

		 void SetTexture(Texture* texture) { mTexture = texture; }
		 void SetSize(Vector2 size) { mSize = size; }
		 Vector2 GetIndex() { return mIndex; }
		 void SetIndex(Vector2 index) { mIndex = index; }

	public:
		static Vector2 TileSize;
		static Vector2 OriginTileSize;
		static Vector2 SelectedIndex;

	private:
		Texture* mTexture;
		Vector2 mTileSize;
		Vector2 mSize;
		Vector2 mIndex;
	};

}




