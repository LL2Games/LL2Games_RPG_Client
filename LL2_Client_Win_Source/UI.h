#pragma once
#include "CommonInclude.h"

class stbD2DRenderer;
class UI
{
public:
	UI() { mActive = true; };
	virtual ~UI() {};

	virtual void Init();
	virtual void Update();
	virtual void Render(HDC hdc);
	virtual void Render(stbD2DRenderer& renderer);

public:
	void SetActivce(bool active) { mActive = active; }
	bool IsActive() const {return mActive;}
	void Toggle() { mActive = !mActive; }
protected:

	bool mActive;
};

