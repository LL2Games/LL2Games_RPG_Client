#include "stbInput.h"
#include "UIManager.h"
#include "PlayerManager.h"

#define M_PLAYERMANAGER stb::SingletonBase<PlayerManager>::getInstance()
namespace stb
{
	int ASCII[(UINT)eKeyCode::EnumsEnd] =
	{
		'F1','F2','F3','F4','F5','F6','F7','F8','F9','F10','F11','F12',
		'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
		'Z', 'X', 'C', 'V', 'B', 'N', 'M',
		VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
		VK_LBUTTON, VK_MBUTTON, VK_RBUTTON, VK_SPACE, VK_LMENU, VK_LCONTROL,
		VK_INSERT,VK_HOME,VK_PRIOR,VK_DELETE,
		VK_END,VK_NEXT,VK_LSHIFT,
		VK_RETURN, VK_RETURN,
	};

	Input::Input()
		: Keys{}
	{
	}

	Input::~Input()
	{
	}

	void Input::Initialize()
	{
		CreateKeys();
		CreateDefaultBindings();
	}

	void Input::Update()
	{
		for (auto& key : Keys)
		{
			UpdateKeys(key);
		}
	}

	void Input::UpdateKeys(Key& key)
	{
		if (GetFocus())
		{
			if (CheckIsKeyDown(key.keyCode))
			{
				IsKeyDown(key);
			}
			else
			{
				IsKeyUp(key);
			}
		}
		else
		{
			key.State = eKeyState::None;
			key.bPressed = false;
		}
	}

	void Input::CreateKeys()
	{
		Keys.clear();
		Keys.reserve((UINT)eKeyCode::EnumsEnd);

		for (int i = 0; i < (UINT)eKeyCode::EnumsEnd; i++)
		{
			Key key = {};
			key.bPressed = false;
			key.keyCode = (eKeyCode)i;
			key.State = eKeyState::None;

			Keys.push_back(key);
		}
	}

	void Input::CreateDefaultBindings()
	{
		m_keyBindings.clear();

		// 이동 액션 바인딩
		m_keyBindings[eKeyCode::Left] = { eBindType::Action, (int)eActionCode::MoveLeft };
		m_keyBindings[eKeyCode::Right] = { eBindType::Action, (int)eActionCode::MoveRight };
		m_keyBindings[eKeyCode::Up] = { eBindType::Action, (int)eActionCode::MoveUp };
		m_keyBindings[eKeyCode::Down] = { eBindType::Action, (int)eActionCode::MoveDown };


		// 기본 ?????? 바인???
		m_keyBindings[eKeyCode::LALT] = { eBindType::Action, (int)eActionCode::Jump };
		m_keyBindings[eKeyCode::A] = { eBindType::Action, (int)eActionCode::Attack };
		m_keyBindings[eKeyCode::SPACE] = { eBindType::Action, (int)eActionCode::Interact };
		m_keyBindings[eKeyCode::I] = { eBindType::Action, (int)eActionCode::Inventory };
		m_keyBindings[eKeyCode::K] = { eBindType::Action, (int)eActionCode::SkillWindow };
		m_keyBindings[eKeyCode::T] = { eBindType::Action, (int)eActionCode::Trade };
		m_keyBindings[eKeyCode::Z] = { eBindType::Action, (int)eActionCode::PickUp };

		//교환 취소 임시(X)
		m_keyBindings[eKeyCode::X] = { eBindType::Action, (int)eActionCode::TradeCancel };


		// 스킬
		m_keyBindings[eKeyCode::Q] = { eBindType::Skill, (int)eSkillCode::Knight_Slash };

		// m_keyBindings[eKeyCode::R] = { eBindType::Item, 2000000 };

		CreateDefaultBindingQuickSlot();
	}

	void Input::CreateDefaultBindingQuickSlot()
	{
		m_keyBindings[eKeyCode::LShift] = { eBindType::QuickSlot, 0 };
		m_keyBindings[eKeyCode::Insert] = { eBindType::QuickSlot, 1 };
		m_keyBindings[eKeyCode::Home]	= { eBindType::QuickSlot, 2 };
		m_keyBindings[eKeyCode::PageUp] = { eBindType::QuickSlot, 3 };

		m_keyBindings[eKeyCode::Num1] = { eBindType::QuickSlot, 4 };
		m_keyBindings[eKeyCode::Num2] = { eBindType::QuickSlot, 5 };
		m_keyBindings[eKeyCode::Num3] = { eBindType::QuickSlot, 6 };
		m_keyBindings[eKeyCode::Num4] = { eBindType::QuickSlot, 7 };
		m_keyBindings[eKeyCode::Num5] = { eBindType::QuickSlot, 8 };
		m_keyBindings[eKeyCode::Num6] = { eBindType::QuickSlot, 9 };
		m_keyBindings[eKeyCode::Num7] = { eBindType::QuickSlot, 10 };
		m_keyBindings[eKeyCode::Num8] = { eBindType::QuickSlot, 11 };
		m_keyBindings[eKeyCode::Num9] = { eBindType::QuickSlot, 12 };
		m_keyBindings[eKeyCode::Num0] = { eBindType::QuickSlot, 13 };

		//m_keyBindings[eKeyCode::I] = { eBindType::QuickSlot, 14 };
		m_keyBindings[eKeyCode::O] = { eBindType::QuickSlot, 15 };

		m_keyBindings[eKeyCode::LCONTROL] = { eBindType::QuickSlot, 16 };
		m_keyBindings[eKeyCode::Delete] = { eBindType::QuickSlot, 17 };
		m_keyBindings[eKeyCode::End] = { eBindType::QuickSlot, 18 };
		m_keyBindings[eKeyCode::PageDown] = { eBindType::QuickSlot, 19 };

		//m_keyBindings[eKeyCode::Q] = { eBindType::QuickSlot, 20 };
		m_keyBindings[eKeyCode::W] = { eBindType::QuickSlot, 21 };
		m_keyBindings[eKeyCode::E] = { eBindType::QuickSlot, 22 };
		m_keyBindings[eKeyCode::R] = { eBindType::QuickSlot, 23 };
		//m_keyBindings[eKeyCode::T] = { eBindType::QuickSlot, 24 };
		//m_keyBindings[eKeyCode::A] = { eBindType::QuickSlot, 25 };
		m_keyBindings[eKeyCode::S] = { eBindType::QuickSlot, 26 };
		m_keyBindings[eKeyCode::D] = { eBindType::QuickSlot, 27 };
		m_keyBindings[eKeyCode::F] = { eBindType::QuickSlot, 28 };
		m_keyBindings[eKeyCode::G] = { eBindType::QuickSlot, 29 };
		//m_keyBindings[eKeyCode::K] = { eBindType::QuickSlot, 30 };
		m_keyBindings[eKeyCode::L] = { eBindType::QuickSlot, 31 };
	}

	bool Input::GetPressedBind(KeyBindInfo& outBindInfo)
	{
		for (int i = 0; i < (UINT)eKeyCode::EnumsEnd; i++)
		{
			eKeyCode code = (eKeyCode)i;

			if (GetKeyDown(code) == false)
				continue;

			auto it = m_keyBindings.find(code);
			if (it == m_keyBindings.end())
				continue;

			outBindInfo = it->second;
			return true;
		}

		return false;
	}


	void Input::BindKey(eKeyCode code, const KeyBindInfo& bindInfo)
	{
		m_keyBindings[code] = bindInfo;
	}

	void Input::UnbindKey(eKeyCode code)
	{
		auto it = m_keyBindings.find(code);
		if (it != m_keyBindings.end())
		{
			m_keyBindings.erase(it);
		}
	}

	bool Input::TryGetBindInfo(eKeyCode code, KeyBindInfo& outBindInfo) const
	{
		auto it = m_keyBindings.find(code);
		if (it == m_keyBindings.end())
		{
			return false;
		}

		outBindInfo = it->second;
		return true;
	}

	void Input::IsKeyDown(Input::Key& key)
	{
		if (key.bPressed == true)
		{
			key.State = eKeyState::Pressed;
		}
		else
		{
			key.State = eKeyState::Down;
		}

		key.bPressed = true;
	}

	void Input::IsKeyUp(Input::Key& key)
	{
		if (key.bPressed == true)
		{
			key.State = eKeyState::Up;
		}
		else
		{
			key.State = eKeyState::None;
		}

		key.bPressed = false;
	}

	bool Input::CheckIsKeyDown(eKeyCode code)
	{
		return GetAsyncKeyState(ASCII[(UINT)code]) & 0x8000;
	}

	bool Input::GetAction(eActionCode action)
	{
		for (const auto& pair : m_keyBindings)
		{
			if (pair.second.type != eBindType::Action)
				continue;

			if (pair.second.value != (int)action)
				continue;

			if (GetKey(pair.first))
				return true;
		}

		return false;
	}

	bool Input::GetActionDown(eActionCode action)
	{
		for (const auto& pair : m_keyBindings)
		{
			if (pair.second.type != eBindType::Action)
				continue;

			if (pair.second.value != (int)action)
				continue;

			if (GetKeyDown(pair.first))
				return true;
		}

		return false;
	}

	bool Input::GetSkillDown(eSkillCode skill)
	{
		for (const auto& pair : m_keyBindings)
		{
			if (pair.second.type != eBindType::Skill)
				continue;

			if (pair.second.value != (int)skill)
				continue;

			if (GetKeyDown(pair.first))
				return true;
		}

		return false;
	}

	bool Input::GetActionUp(eActionCode action)
	{
		for (const auto& pair : m_keyBindings)
		{
			if (pair.second.type != eBindType::Action)
				continue;

			if (pair.second.value != (int)action)
				continue;

			if (GetKeyUp(pair.first))
				return true;
		}

		return false;
	}
	eKeyCode Input::FindKeyByQuickSlotIndex(int slotIndex) const
	{
		for (const auto& pair : m_keyBindings)
		{
			const eKeyCode keyCode = pair.first;
			const KeyBindInfo& binding = pair.second;

			if (binding.type == eBindType::QuickSlot &&
				binding.value == slotIndex)
			{
				return keyCode;
			}
		}

		return eKeyCode::EnumsEnd;
	}
	std::wstring Input::KeyCodeToWString(eKeyCode keyCode) const
	{
		switch (keyCode)
		{
		case eKeyCode::Num1: return L"1";
		case eKeyCode::Num2: return L"2";
		case eKeyCode::Num3: return L"3";
		case eKeyCode::Num4: return L"4";
		case eKeyCode::Num5: return L"5";
		case eKeyCode::Num6: return L"6";
		case eKeyCode::Num7: return L"7";
		case eKeyCode::Num8: return L"8";
		case eKeyCode::Num9: return L"9";
		case eKeyCode::Num0: return L"0";

		case eKeyCode::Q: return L"Q";
		case eKeyCode::W: return L"W";
		case eKeyCode::E: return L"E";
		case eKeyCode::R: return L"R";
		case eKeyCode::T: return L"T";
		case eKeyCode::A: return L"A";
		case eKeyCode::S: return L"S";
		case eKeyCode::D: return L"D";
		case eKeyCode::F: return L"F";
		case eKeyCode::G: return L"F";
		case eKeyCode::K: return L"F";
		case eKeyCode::L: return L"F";

		case eKeyCode::LShift:		return L"Shift";
		case eKeyCode::Insert:		return L"Ins";
		case eKeyCode::Home:		return L"Hm";
		case eKeyCode::PageUp:		return L"Pup";
		case eKeyCode::PageDown:	return L"Pdn";
		case eKeyCode::LCONTROL:	return L"Ctrl";
		case eKeyCode::Delete:		return L"Del";
		case eKeyCode::End:			return L"End";
		default:
			return L"";
		}
	}
}
