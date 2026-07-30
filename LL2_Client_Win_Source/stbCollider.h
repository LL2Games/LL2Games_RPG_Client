#pragma once
#include "stbComponent.h"

class stbD2DRenderer;


namespace stb
{
	class Collider : public Component
	{
	public:
		Collider();
		~Collider();
		virtual void Initialize() ;
		virtual void Update();
		virtual void LateUpdate();
		virtual void Render(HDC hdc) ;
		virtual void Render(stbD2DRenderer& renderer);


		virtual void OnCollisionEnter(Collider* other);
		virtual void OnCollisionStay(Collider* other);
		virtual void OnCollisionEnd(Collider* other);

		UINT32 GetID() { return CollisionID; }

		void SetOffset(Vector2 offset) { mOffset = offset; }
		Vector2 GetOffset() { return mOffset; }

		void SetSize(Vector2 size) { mSize = size; }
		Vector2 GetSize() { return mSize; }

		eColliderType GetColliderType() { return mType; }
		void SetColliderType(eColliderType type) { mType = type; }



	private:
		static UINT32 CollisionID;

		UINT32 mID;
		Vector2 mOffset;
		Vector2 mSize;
		eColliderType mType = eColliderType::EnumsEnd;

	};

}


