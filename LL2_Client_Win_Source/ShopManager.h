#pragma once

#include "stbSingletonBase.h"
#include "ShopData.h"

#include <optional>
#include <utility>

class ShopManager : public stb::SingletonBase<ShopManager>
{
public:
    void Open(ShopOpenResult result)
    {
        m_currentShop = std::move(result);
    }

    void Close()
    {
        m_currentShop.reset();
    }

    bool IsOpen() const
    {
        return m_currentShop.has_value();
    }

    // Close/Open 호출 후에는 기존 포인터를 다시 사용하지 않는다.
    const ShopOpenResult* GetCurrentShop() const
    {
        return m_currentShop ? &*m_currentShop : nullptr;
    }

private:
    std::optional<ShopOpenResult> m_currentShop;
};
