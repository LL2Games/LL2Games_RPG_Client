#pragma once
#include "stbSingletonBase.h"
#include "CommonInclude.h"

namespace stb
{
	enum class eKeyCode
	{
		F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
		Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
		Q, W, E, R, T, Y, U, I, O, P,
		A, S, D, F, G, H, J, K, L,
		Z, X, C, V, B, N, M,
		Left, Right, Up, Down,
		LButton, MButton, RButton, SPACE,
		LALT, LCONTROL, Insert,Home,PageUp,
		Delete,End,PageDown, 
		LShift,
		Enter, Return,
		EnumsEnd,
	};

	enum class eKeyState
	{
		Down,
		Pressed,
		Up,
		None,

	};

	enum class eActionCode
	{
		None,
		Interact,
		MoveLeft,
		MoveRight,
		MoveUp,
		MoveDown,
		Jump,
		Attack,
		PickUp,
		Inventory,
		CharacterInfo,
		SkillWindow,
		QuestWindow,
		WorldMap,
		EnumsEnd,
		Trade,
		TradeCancel,
	};

	enum class eSkillCode
	{
		None = 0,
		Knight_Slash = 20001,
		Archer_Shot = 21001,
	};

	enum class eBindType
	{
		None,
		Action,   // Interact 媛숈?? 寃?
		Skill,
		Item,
		UI,
		QuickSlot,
	};

	struct KeyBindInfo
	{
		eBindType type;
		int value;
	};

	struct EnumClassHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
	};

	class Input : public SingletonBase<Input>
	{

	public:
		struct Key
		{
			eKeyCode keyCode;
			eKeyState State;
			bool bPressed;
		};

		Input();
		~Input();

		__forceinline bool GetKeyDown(eKeyCode eCode) { return Keys[(UINT)eCode].State == eKeyState::Down; }
		__forceinline bool GetKeyUp(eKeyCode eCode) { return Keys[(UINT)eCode].State == eKeyState::Up; }
		__forceinline bool GetKey(eKeyCode eCode) { return Keys[(UINT)eCode].State == eKeyState::Pressed; }


		void Initialize();
		void Update();
		void UpdateKeys(Key& key);
		void CreateKeys();

		void IsKeyDown(Input::Key& key);
		void IsKeyUp(Input::Key& key);

		bool CheckIsKeyDown(eKeyCode code);

		bool GetAction(eActionCode action);
		bool GetActionDown(eActionCode action);
		bool GetActionUp(eActionCode action);

		bool GetSkillDown(eSkillCode skill);

		eKeyCode FindKeyByQuickSlotIndex(int slotIndex) const;
		std::wstring KeyCodeToWString(eKeyCode keyCode) const;
	public:
		void CreateDefaultBindings();
		void CreateDefaultBindingQuickSlot();
		bool GetPressedBind(KeyBindInfo& outBindInfo);
	
		void BindKey(eKeyCode code, const KeyBindInfo& bindInfo);
		void UnbindKey(eKeyCode code);
		bool TryGetBindInfo(eKeyCode code, KeyBindInfo& outBindInfo) const;

	private:
		std::vector<Key> Keys;
		
		std::unordered_map<eKeyCode, KeyBindInfo, EnumClassHash> m_keyBindings;


	};

}

