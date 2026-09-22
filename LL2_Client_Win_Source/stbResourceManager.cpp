#include "stbD2DRenderer.h"
#include "stbTexture.h"
#include "stbResourceManager.h"
#include <nlohmann/json.hpp>
#include "stbAudioClip.h"

stb::ResourceManager::ResourceManager()
{
}

stb::ResourceManager::~ResourceManager()
{
}

void stb::ResourceManager::LoadAllD2D(stbD2DRenderer& renderer)
{
    for (auto& pair : mResources)
    {
        stb::Texture* tex = dynamic_cast<stb::Texture*>(pair.second);
        if (tex) tex->LoadD2D(renderer);
    }
}

void stb::ResourceManager::LoadMonsterTextures()
{
	std::filesystem::path root = L"Resources_Woodland\\Monster";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}
			

		if (entry.path().extension() != ".png")
		{
			continue;
		}
			

		std::filesystem::path relative = std::filesystem::relative(entry.path(), L"Resources_Woodland");

		std::wstring key = relative.replace_extension(L"").generic_wstring();

		OutputDebugStringW(key.c_str());
		OutputDebugStringA("\n");
		Load<Texture>(key, entry.path().wstring());
	}
}

void stb::ResourceManager::LoadPlayerTextures()
{
	std::filesystem::path root = L"Resources_Woodland\\Character";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}


		if (entry.path().extension() != ".png")
		{
			continue;
		}


		std::filesystem::path relative = std::filesystem::relative(entry.path(), L"Resources_Woodland");

		std::wstring key = relative.replace_extension(L"").generic_wstring();

		OutputDebugStringW(key.c_str());
		OutputDebugStringA("\n");
		Load<Texture>(key, entry.path().wstring());
	}
}

void stb::ResourceManager::LoadItemTextures()
{
	std::filesystem::path root = L"Resources_Woodland\\Item";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}

			
		if (entry.path().extension() != ".png")
		{
			continue;
		}

		std::wstring key = entry.path().stem().wstring();;

		OutputDebugStringW(key.c_str());
		OutputDebugStringA("\n");
		Load<Texture>(key, entry.path().wstring());
	}
}

void stb::ResourceManager::LoadHealthBarTextures()
{
	Load<Texture>(L"HealthBar_Background", L"Resources_Woodland\\UI\\HealthBar\\HealthBar_backgrnd.png");
	Load<Texture>(L"HealthBar_hpLayer", L"Resources_Woodland\\UI\\HealthBar\\HealthBar_hp_layer_0.png");
	Load<Texture>(L"HealthBar_mpLayer", L"Resources_Woodland\\UI\\HealthBar\\HealthBar_mp_layer_0.png");
	//Load<Texture>(L"HealthBar_guage", L"Resources_Woodland\\UI\\HealthBar\\main.status.normal.gauge.number.png");

	Load<Texture>(L"HealthBar_0", L"Resources_Woodland\\UI\\HealthBar\\number_0.png");
	Load<Texture>(L"HealthBar_1", L"Resources_Woodland\\UI\\HealthBar\\number_1.png");
	Load<Texture>(L"HealthBar_2", L"Resources_Woodland\\UI\\HealthBar\\number_2.png");
	Load<Texture>(L"HealthBar_3", L"Resources_Woodland\\UI\\HealthBar\\number_3.png");
	Load<Texture>(L"HealthBar_4", L"Resources_Woodland\\UI\\HealthBar\\number_4.png");
	Load<Texture>(L"HealthBar_5", L"Resources_Woodland\\UI\\HealthBar\\number_5.png");
	Load<Texture>(L"HealthBar_6", L"Resources_Woodland\\UI\\HealthBar\\number_6.png");
	Load<Texture>(L"HealthBar_7", L"Resources_Woodland\\UI\\HealthBar\\number_7.png");
	Load<Texture>(L"HealthBar_8", L"Resources_Woodland\\UI\\HealthBar\\number_8.png");
	Load<Texture>(L"HealthBar_9", L"Resources_Woodland\\UI\\HealthBar\\number_9.png");
	Load<Texture>(L"HealthBar_/", L"Resources_Woodland\\UI\\HealthBar\\number_slash.png");
}

void stb::ResourceManager::LoadExpTextures()
{
	Load<Texture>(L"1366_expBar_backLayer", L"Resources_Woodland\\UI\\Exp\\1366_layer_back.png");
	Load<Texture>(L"1366_expBar_gaugeLayer", L"Resources_Woodland\\UI\\Exp\\1366_layer_gauge.png");
	/*Load<Texture>(L"1228_expBar_backLayer", L"Resources_Woodland\\UI\\Exp\\1228_layer_back.png");
	Load<Texture>(L"1228_expBar_gaugeLayer", L"Resources_Woodland\\UI\\Exp\\1228_layer_gauge.png");
	Load<Texture>(L"1280_expBar_backLayer", L"Resources_Woodland\\UI\\Exp\\1280_layer_back.png");
	Load<Texture>(L"1280_expBar_gaugeLayer", L"Resources_Woodland\\UI\\Exp\\1280_layer_gauge.png");
	Load<Texture>(L"1920_expBar_backLayer", L"Resources_Woodland\\UI\\Exp\\1920_layer_back.png");
	Load<Texture>(L"1920_expBar_gaugeLayer", L"Resources_Woodland\\UI\\Exp\\1920_layer_gauge.png");*/


}


void stb::ResourceManager::LoadLevelTextures()
{
	Load<Texture>(L"Level_Background",			L"Resources_Woodland\\UI\\Level\\namePlate_backgrnd.png");
	Load<Texture>(L"Level_lvLayer",				L"Resources_Woodland\\UI\\Level\\namePlate_layer_lv.png");
	Load<Texture>(L"Level_0",					L"Resources_Woodland\\UI\\Level\\number_0.png");
	Load<Texture>(L"Level_1",					L"Resources_Woodland\\UI\\Level\\number_1.png");
	Load<Texture>(L"Level_2",					L"Resources_Woodland\\UI\\Level\\number_2.png");
	Load<Texture>(L"Level_3",					L"Resources_Woodland\\UI\\Level\\number_3.png");
	Load<Texture>(L"Level_4",					L"Resources_Woodland\\UI\\Level\\number_4.png");
	Load<Texture>(L"Level_5",					L"Resources_Woodland\\UI\\Level\\number_5.png");
	Load<Texture>(L"Level_6",					L"Resources_Woodland\\UI\\Level\\number_6.png");
	Load<Texture>(L"Level_7",					L"Resources_Woodland\\UI\\Level\\number_7.png");
	Load<Texture>(L"Level_8",					L"Resources_Woodland\\UI\\Level\\number_8.png");
	Load<Texture>(L"Level_9",					L"Resources_Woodland\\UI\\Level\\number_9.png");
}

void stb::ResourceManager::LoadInventoryTextures()
{
	Load<Texture>(L"Inventory_normal", L"Resources_Woodland\\UI\\Inventory\\Inventory_backgrnd.png");
	Load<Texture>(L"Inventory_full", L"Resources_Woodland\\UI\\Inventory\\Inventory.FullBackgrnd.png");

	Load<Texture>(L"Inventory_equip_normal",	L"Resources_Woodland\\UI\\Inventory\\category_normal_0_.png");
	Load<Texture>(L"Inventory_consume_normal",	L"Resources_Woodland\\UI\\Inventory\\category_normal_1_.png");
	Load<Texture>(L"Inventory_etc_normal",		L"Resources_Woodland\\UI\\Inventory\\category_normal_2_.png");
	Load<Texture>(L"Inventory_setup_normal",	L"Resources_Woodland\\UI\\Inventory\\category_normal_3_.png");
	Load<Texture>(L"Inventory_cash_normal",		L"Resources_Woodland\\UI\\Inventory\\category_normal_4_.png");
	Load<Texture>(L"Inventory_cosmetic_normal", L"Resources_Woodland\\UI\\Inventory\\category_normal_5_.png");

	Load<Texture>(L"Inventory_equip_selected", L"Resources_Woodland\\UI\\Inventory\\category_selected_0.png");
	Load<Texture>(L"Inventory_consume_selected", L"Resources_Woodland\\UI\\Inventory\\category_selected_1.png");
	Load<Texture>(L"Inventory_etc_selected", L"Resources_Woodland\\UI\\Inventory\\category_selected_2.png");
	Load<Texture>(L"Inventory_setup_selected", L"Resources_Woodland\\UI\\Inventory\\category_selected_3.png");
	Load<Texture>(L"Inventory_cash_selected", L"Resources_Woodland\\UI\\Inventory\\category_selected_4.png");
	Load<Texture>(L"Inventory_cosmetic_selected", L"Resources_Woodland\\UI\\Inventory\\category_selected_5.png");

	Load<Texture>(L"Inventory_full_equip_normal",		L"Resources_Woodland\\UI\\Inventory\\full_category_normal_0_.png");
	Load<Texture>(L"Inventory_full_consume_normal",		L"Resources_Woodland\\UI\\Inventory\\full_category_normal_1_.png");
	Load<Texture>(L"Inventory_full_etc_normal",			L"Resources_Woodland\\UI\\Inventory\\full_category_normal_2_.png");
	Load<Texture>(L"Inventory_full_setup_normal",		L"Resources_Woodland\\UI\\Inventory\\full_category_normal_3_.png");
	Load<Texture>(L"Inventory_full_cash_normal",		L"Resources_Woodland\\UI\\Inventory\\full_category_normal_4_.png");
	Load<Texture>(L"Inventory_full_cosmetic_normal",	L"Resources_Woodland\\UI\\Inventory\\full_category_normal_5_.png");

	Load<Texture>(L"Inventory_full_equip_selected", L"Resources_Woodland\\UI\\Inventory\\full_category_selected_0.png");
	Load<Texture>(L"Inventory_full_consume_selected", L"Resources_Woodland\\UI\\Inventory\\full_category_selected_1.png");
	Load<Texture>(L"Inventory_full_etc_selected", L"Resources_Woodland\\UI\\Inventory\\full_category_selected_2.png");
	Load<Texture>(L"Inventory_full_setup_selected", L"Resources_Woodland\\UI\\Inventory\\full_category_selected_3.png");
	Load<Texture>(L"Inventory_full_cash_selected", L"Resources_Woodland\\UI\\Inventory\\full_category_selected_4.png");
	Load<Texture>(L"Inventory_full_cosmetic_selected", L"Resources_Woodland\\UI\\Inventory\\full_category_selected_5.png");

	Load<Texture>(L"Inventory_full_normal", L"Resources_Woodland\\UI\\Inventory\\full_normal_0.png");
	Load<Texture>(L"Inventory_full_mouseOver", L"Resources_Woodland\\UI\\Inventory\\full_mouseOver_0.png");
	Load<Texture>(L"Inventory_full_pressed", L"Resources_Woodland\\UI\\Inventory\\full_pressed0.png");

	Load<Texture>(L"Inventory_min_normal", L"Resources_Woodland\\UI\\Inventory\\min_normal_0.png");
	Load<Texture>(L"Inventory_min_mouseOver", L"Resources_Woodland\\UI\\Inventory\\min_mouseOver_0.png");
	Load<Texture>(L"Inventory_min_pressed", L"Resources_Woodland\\UI\\Inventory\\min_pressed_0.png");


	Load<Texture>(L"Inventory_close_normal", L"Resources_Woodland\\UI\\Inventory\\close_normal_0.png");
	Load<Texture>(L"Inventory_close_mouseOver", L"Resources_Woodland\\UI\\Inventory\\close_mouseOver_0.png");
	Load<Texture>(L"Inventory_close_pressed", L"Resources_Woodland\\UI\\Inventory\\close_pressed_0.png");

}

void stb::ResourceManager::LoadTradeTextures()
{
	Load<Texture>(L"Trade_normal", L"Resources_Woodland\\UI\\ItemTrade\\backgrnd.png");
	Load<Texture>(L"Trade_full", L"Resources_Woodland\\UI\\ItemTrade\\FullBackgrnd.png");

	Load<Texture>(L"Trade_button_trade_normal", L"Resources_Woodland\\UI\\ItemTrade\\button_trade_normal.png");
	Load<Texture>(L"Trade_button_trade_mouseOver", L"Resources_Woodland\\UI\\ItemTrade\\button_trade_mouseOver.png");
	Load<Texture>(L"Trade_button_trade_checked", L"Resources_Woodland\\UI\\ItemTrade\\button_trade.checked.png");

	Load<Texture>(L"Trade_button_confirm_normal", L"Resources_Woodland\\UI\\ItemTrade\\itemTrade.AutoBuild.button_confirm.normal.0.png");
	//Load<Texture>(L"Trade_layer_confirm_me", L"Resources_Woodland\\UI\\ItemTrade\\itemTrade.AutoBuild.layer_confirmMe.png");
}

void stb::ResourceManager::LoadBGMAudioClips()
{
	Load<AudioClip>(L"BGM_Forest_ground_1", L"Resources_Woodland\\BGM\\forest\\041415calmbgm.mp3");
}


void stb::ResourceManager::LoadMapTextures()
{
	Load<Texture>(L"Forest_ground_1", L"Resources_Woodland\\Background\\forest\\forest_ground_1.png");
	Load<Texture>(L"Forest_ground_2", L"Resources_Woodland\\Background\\forest\\forest_ground_2.png");
	Load<Texture>(L"Forest_ground_3", L"Resources_Woodland\\Background\\forest\\forest_ground_3.png");
}

void stb::ResourceManager::LoadQuickSlotTextures()
{
	Load<Texture>(L"quickslot_background", L"Resources_Woodland\\UI\\quickslot\\backgrnd.png");
}

void stb::ResourceManager::LoadPortalTextures()
{
	Load<Texture>(L"ForestPortal", L"Resources_Woodland\\Portal\\forest_portal.png");
}


void stb::ResourceManager::LoadStatTextures()
{
	Load<Texture>(L"Stat_background", L"Resources_Woodland\\UI\\Stat\\Stat_background.png");
	Load<Texture>(L"plus_button", L"Resources_Woodland\\UI\\Stat\\plus_button.png");
	Load<Texture>(L"minus_button", L"Resources_Woodland\\UI\\Stat\\minus_button.png");
}

void stb::ResourceManager::LoadProjectileTextures()
{
	std::filesystem::path root = L"Resources_Woodland\\Projectile";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}


		if (entry.path().extension() != ".png")
		{
			continue;
		}


		std::filesystem::path relative = std::filesystem::relative(entry.path(), L"Resources_Woodland");

		std::wstring key = relative.replace_extension(L"").generic_wstring();

		OutputDebugStringW(key.c_str());
		OutputDebugStringA("\n");
		Load<Texture>(key, entry.path().wstring());
	}
}
