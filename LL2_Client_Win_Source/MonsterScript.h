#pragma once
#include "CommonInclude.h"
#include "stbScript.h"


class MonsterScript : public stb::Script
{
public:
	void Initialize() override;
	void Update() override;
	void LateUpdate() override;
	void Render(HDC hdc) override;
private:
};

