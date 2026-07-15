#include "stbCollisionManager.h"
#include "stbLayer.h"
#include "stbGameObject.h"
#include "stbScene.h"
#include "stbSceneManager.h"
#include "stbCollider.h"
#include "stbTransform.h"
#include "stbD2DRenderer.h"

#define M_SceneManager stb::SingletonBase<stb::SceneManager>::getInstance()

namespace stb
{
	CollisionManager::CollisionManager()
	{
	}

	CollisionManager::~CollisionManager()
	{
	}
	void CollisionManager::Initailzie()
	{
	}

	void CollisionManager::Update()
	{
		Scene* scene = M_SceneManager->GetActiveScene();

		for (UINT row = 0; row < (UINT)eLayerType::Max; row++)
		{
			for (UINT col = 0; col < (UINT)eLayerType::Max; col++)
			{
				if (mCollisionLayerMatrix[row][col] == true)
				{
					LayerCollosion(scene, (eLayerType)row, (eLayerType)col);
				}
			}
		}

	}
	void CollisionManager::LateUpdate()
	{
	}

	void CollisionManager::Render(HDC hdc)
	{
	}

	void CollisionManager::Render(stbD2DRenderer& renderer)
	{

	}

	void CollisionManager::Clear()
	{
		mCollisionMap.clear();
		mCollisionLayerMatrix->reset();
	}

	void CollisionManager::CollisionLayerCheck(eLayerType left, eLayerType right, bool enable)
	{
		int row = 0;
		int col = 0;

		if (left > right)
		{
			row = (UINT)left;
			col = (UINT)right;
		}
		else
		{
			row = (UINT)right;
			col = (UINT)left;
		}

		mCollisionLayerMatrix[row][col] = enable;
	}

	void CollisionManager::LayerCollosion(Scene* scene, eLayerType left, eLayerType right)
	{
		const std::vector<GameObject*>& lefts = M_SceneManager->GetGameObjects(left);
		const std::vector<GameObject*>& rights = M_SceneManager->GetGameObjects(right);

		for (GameObject* left : lefts)
		{
			Collider* leftCol = left->GetComponent<Collider>();
			if (leftCol == nullptr)
			{
				continue;
			}

			for (GameObject* right : rights)
			{
				Collider* rightCol = right->GetComponent<Collider>();
				if (rightCol == nullptr)
				{
					continue;
				}
				if (left == right)
				{
					continue;
				}

				ColliderCollision(leftCol, rightCol);
			}
		}
	}

	void CollisionManager::ColliderCollision(Collider* left, Collider* right)
	{
		ColliderID id = {};
		id.left = left->GetID();
		id.right = right->GetID();


		auto iter = mCollisionMap.find(id.id);
		if (iter == mCollisionMap.end())
		{
			mCollisionMap.insert(std::make_pair(id.id, false));
			iter = mCollisionMap.find(id.id);
		}


		if (Intersect(left, right))
		{
			if (iter->second == false)
			{
				left->OnCollisionEnter(right);
				right->OnCollisionEnter(left);
				iter->second = true;
			}
			else
			{
				left->OnCollisionStay(right);
				right->OnCollisionStay(left);
			}
		}
		else
		{
			if (iter->second == true)
			{
				left->OnCollisionEnd(right);
				right->OnCollisionEnd(left);

				iter->second = false;
			}
		}

	}

	bool CollisionManager::Intersect(Collider* left, Collider* right)
	{
		Transform* leftTr = left->GetOwner()->GetComponent<Transform>();
		Transform* rightTr = right->GetOwner()->GetComponent<Transform>();

		Vector2 leftPos = leftTr->GetPosition();
		Vector2 rightPos = rightTr->GetPosition();

		Vector2 leftSize = left->GetSize() * 100.0f;
		Vector2 rightSize = right->GetSize() * 100.0f;

		Vector2 leftCirclePos = leftPos + (leftSize / 2.0f);
		Vector2 rightCirclePos = rightPos + (rightSize / 2.0f);

		eColliderType leftType = left->GetColliderType();
		eColliderType rightType = right->GetColliderType();


		if (leftType == eColliderType::Rect2D && rightType == eColliderType::Rect2D)
		{
			if(fabs(leftPos.x- rightPos.x) < fabs(leftSize.x / 2.0f + rightSize.x / 2.0f)
				&& fabs(leftPos.y - rightPos.y) < fabs(leftSize.y / 2.0f + rightSize.y / 2.0f))
			{
				return true;
			}
		}

		else if (leftType == eColliderType::Circle2D && rightType == eColliderType::Circle2D)
		{
			float distance = (leftCirclePos - rightCirclePos).length();

			if (distance <= (leftSize.x / 2.0f + rightPos.x / 2.0f))
			{
				return true;
			}


		}


		else if (leftType == eColliderType::Circle2D && rightType == eColliderType::Rect2D
			|| leftType == eColliderType::Rect2D && rightType == eColliderType::Circle2D)
		{

			Vector2 CirclePos;
			Vector2 RectPos;
			Vector2 RectSize;
			float radius;


			if (leftType == eColliderType::Circle2D)
			{
				CirclePos = leftPos + (leftPos / 2.0f);
				radius = leftSize.x / 2.0f;
				RectPos = rightPos;
				RectSize = rightSize;
			}
			else
			{
				CirclePos = rightPos + (rightPos / 2.0f);
				radius = rightSize.x / 2.0f;
				RectPos = leftPos;
				RectSize = leftSize;
			}


			RECT reSizeRect = { RectPos.x - radius, RectPos.y - radius, RectPos.x + radius, RectPos.y + radius };

			if (reSizeRect.left < CirclePos.x && CirclePos.x < reSizeRect.right
				&& reSizeRect.top < CirclePos.y && CirclePos.y < reSizeRect.bottom)
			{
				return true;
			}
			else
			{
				Vector2 topLeft = { RectPos.x , RectPos.y };
				Vector2 topRight = { RectPos.x + RectSize.x , RectPos.y };
				Vector2 bottomLeft = { RectPos.x, RectPos.y + RectSize.y };
				Vector2 bottomRight = { RectPos.x + RectSize.x , RectPos.y + RectSize.y };

				Vector2 corners[] = { topLeft, topRight, bottomLeft, bottomRight };
				for (int i = 0; i < 4; i++)
				{
					float distance = (corners[i] - leftCirclePos).length();
					if (distance <= leftSize.x / 2.0f)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

}
