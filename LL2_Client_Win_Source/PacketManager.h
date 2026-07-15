#pragma once
#include "CommonInclude.h"
#include "stbSingletonBase.h"

class PacketManager : public stb::SingletonBase<PacketManager>
{
public:
	PacketManager() {};
	~PacketManager() {};
	bool RegisterAllHandlers();

private:
};

