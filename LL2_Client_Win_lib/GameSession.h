#pragma once
#include "..\\LL2_Client_Win_Source\\stbSingletonBase.h"

class GameSession : public stb::SingletonBase<GameSession>
{
public:
	void InitializeOnce();
	void EnsurePersistentObjects();

private:
	bool m_initialized = false;

};

