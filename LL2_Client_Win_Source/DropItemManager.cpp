#include "DropItemManager.h"
#include "stbD2DRenderer.h"
#include "stbResourceManager.h"
#include "stbTexture.h"
#include "stbRender.h"
#include "ItemPacketHandler.h"

#define M_RESOURCEMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()



void DropItemManager::Render(stbD2DRenderer& renderer)
{
    for (const auto& [id, data] : m_dropItems)
    {
        std::wstring key = std::to_wstring(data.itemId);
        std::string DebugMsg;
     

        stb::Texture* itemTex = M_RESOURCEMANAGER->Find<stb::Texture>(key);
        if (itemTex == nullptr)
            continue;

        ID2D1Bitmap* bitmap = itemTex->GetD2DBitmap();
        if (bitmap == nullptr)
            continue;

        float itemWidth = static_cast<float>(itemTex->GetWidth());
        float itemHeight = static_cast<float>(itemTex->GetHeight());

        stb::Vector2 renderPos = data.itemPos;

        if (stb::render::mainCamera != nullptr)
        {
            renderPos = stb::render::mainCamera->CalculatePosition(renderPos);
        }

        float itemX = renderPos.x - itemWidth / 2.0f;
        float itemY = renderPos.y - itemHeight / 2.0f;

        renderer.DrawBitmap(
            bitmap,
            itemX,
            itemY,
            itemWidth,
            itemHeight,
            1.0f
        );
    }
}

void DropItemManager::Update()
{

}


void DropItemManager::AddDropItem(const DropItemData& dropItemData)
{
	auto item = m_dropItems.find(dropItemData.dropId);

	if (item != m_dropItems.end())
	{
		return;
	}

	m_dropItems.emplace(dropItemData.dropId, dropItemData);
}

DropItemData* DropItemManager::GetDropItem(int dropId)
{
	auto item = m_dropItems.find(dropId);

	if (item == m_dropItems.end())
	{
		return nullptr;
	}

	return &item->second;
}

void DropItemManager::RequestPickup(const Vector2& playerPos)
{
    int dropId = FindPickupItem(playerPos);

    if (dropId < 0)
        return;

    ItemPacketHandler::SendPickupDropItem(dropId);
}

int64_t DropItemManager::FindPickupItem(const Vector2& playerPos)
{
    constexpr float pickupRange = 50.0f;
    float nearestDistSq = pickupRange * pickupRange;

    int nearestDropId = -1;

    for (const auto& [dropId, data] : m_dropItems)
    {
        float dx = data.itemPos.x - playerPos.x;
        float dy = data.itemPos.y - playerPos.y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= nearestDistSq)
        {
            nearestDistSq = distSq;
            nearestDropId = dropId;
        }
    }

    return nearestDropId;
}


void DropItemManager::RemoveDropItem(int dropId)
{
    auto item = m_dropItems.find(dropId);

    if (item == m_dropItems.end())
    {
        std::string DebugMsg = "m_dropItems Cant find item\n";
        OutputDebugStringA(DebugMsg.c_str());
        return;
    }

    m_dropItems.erase(item);
}


