#include "stbDontDestroyOnLoad.h"
#include "stbGameObject.h"


namespace stb
{
	DontDestroyOnLoad::DontDestroyOnLoad()
		:mPlayer(nullptr)
	{

	}

	DontDestroyOnLoad::~DontDestroyOnLoad()
	{

	}

	void DontDestroyOnLoad::Initialize()
	{
		Scene::Initialize();
	}

	void DontDestroyOnLoad::Update()
	{
		Scene::Update();
	}

	void DontDestroyOnLoad::LateUpdate()
	{
		Scene::LateUpdate();
	}

	void DontDestroyOnLoad::Render(HDC hdc)
	{
		Scene::Render(hdc);
	}

	void DontDestroyOnLoad::OnExit()
	{
		Scene::OnExit();
	}

	void DontDestroyOnLoad::OnEnter()
	{
		Scene::OnEnter();
	}
}