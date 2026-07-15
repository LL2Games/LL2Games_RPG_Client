#include "stbAnimator.h"
#include "stbTransform.h"
#include "stbResourceManager.h"
#include "stbD2DRenderer.h"
#include <filesystem>

#define M_REMANAGER stb::SingletonBase<stb::ResourceManager>::getInstance()




namespace stb
{
	
	Animator::Animator()
		:Component(eComponentType::Animator)
		, mAnimations({})
		, mActiveAnimation(nullptr)
		, mbLoop(false)
		, mbCompleteEventCalled(false)
	{

	}

	Animator::~Animator()
	{

	}

	void Animator::Initialize()
	{
	}

	void Animator::Update()
	{
		if (mActiveAnimation == nullptr)
			return;

		mActiveAnimation->Update();

		if (mActiveAnimation->IsComplete())
		{
			if (!mbCompleteEventCalled)
			{
				InvokeCompleteEvent(mActiveAnimation->GetName());
				mbCompleteEventCalled = true;
			}

			if (mbLoop)
			{
				mActiveAnimation->Reset();
				mbCompleteEventCalled = false;
			}
		}
	}

	void Animator::LateUpdate()
	{
	}

	void Animator::Render(HDC hdc)
	{
		if (mActiveAnimation)
			mActiveAnimation->Render(hdc);
	}

	void Animator::Render(stbD2DRenderer& renderer)
	{
		if (mActiveAnimation) {
			mActiveAnimation->Render(renderer, m_flipX);
		}
			
	}

	void Animator::CreateAnimation(const std::wstring& name
		, Texture* spriteTexture
		, Vector2 leftTop
		, Vector2 size
		, Vector2 offset
		, UINT spriteLength
		, float duration)
	{
		Animation* animation = nullptr;

		animation = FindAnimation(name);
		if (animation)
		{
			return;
		}

		animation = new Animation();
		animation->SetName(name);
		animation->CreateAnimation(name, spriteTexture, leftTop, size, offset, spriteLength, duration);

		animation->SetAnimator(this);

		Events* events = new Events();

		mAnimations.insert(make_pair(name, animation));
		mEvents.insert(make_pair(name, events));

	}


	void Animator::CreateAnimationByFolder(const std::wstring& name
		, const std::wstring& path
		, Vector2 offset, float duration)
	{
		Animation* animation = nullptr;
		animation = FindAnimation(name);

		if (animation != nullptr)
		{
			return;
		}

		int fileCount = 0;
		std::filesystem::path fs(path);
		std::vector<Texture*> images = {};

		for (auto& p : std::filesystem::recursive_directory_iterator(fs))
		{
			std::wstring fileName = p.path().filename();
			std::wstring filePath = p.path();

			Texture* texture = M_REMANAGER->Load<Texture>(fileName, filePath);
			images.push_back(texture);
			fileCount++;
		}

		UINT sheetWidth = images[0]->GetWidth() * fileCount;
		UINT sheetHeight = images[0]->GetHeight();
		Texture* spriteSheet = Texture::Create(name, sheetWidth, sheetHeight);

		UINT imageWidth = images[0]->GetWidth();
		UINT imageHeight = images[0]->GetHeight();

		for (std::size_t i = 0; i < images.size(); ++i)
		{
			const std::size_t destXSize =
				i * static_cast<std::size_t>(imageWidth);

			if (destXSize >
				static_cast<std::size_t>((std::numeric_limits<int>::max)()))
			{
				throw std::overflow_error("sprite sheet width exceeds int range");
			}

			const int destX = static_cast<int>(destXSize);

			BitBlt(
				spriteSheet->GetHdc(),
				destX,
				0,
				imageWidth,
				imageHeight,
				images[i]->GetHdc(),
				0,
				0,
				SRCCOPY
			);
		}

		CreateAnimation(name, spriteSheet
			, Vector2(0.0f, 0.0f), Vector2(static_cast<float>(imageWidth), static_cast<float>(imageHeight))
			, offset, fileCount, duration);

	}

	void Animator::CreateFrameAnimation(const std::wstring& name, const std::vector<Texture*>& frames, Vector2 origin, Vector2 offset, float duration)
	{
		if (frames.empty())
		{
			std::string DebugMsg = "Frame is Empty \n";
			OutputDebugStringA(DebugMsg.c_str());
			return;
		}
			

		Animation* animation = FindAnimation(name);
		if (animation != nullptr)
			return;

		std::wstring DebugMsg = L"Animation Name : " + name + L"\n";
		OutputDebugStringW(DebugMsg.c_str());

		animation = new Animation();
		animation->SetName(name);
		animation->CreateFrameAnimation(name, frames, origin, offset, duration);
		animation->SetAnimator(this);

		Events* events = new Events();

		mAnimations.insert(std::make_pair(name, animation));
		mEvents.insert(std::make_pair(name, events));

		EventNames eventNames{};
		m_AnimationEventNames.insert(std::make_pair(name, eventNames));
	}

	void Animator::CreateFrameAnimation(const std::wstring& name, const std::vector<Texture*>& frames, Vector2 baseOffset, const std::vector<stb::math::Vector2>& frameOffsets, float duration)
	{
		if (frames.empty())
		{
			std::string DebugMsg = "Frame is Empty \n";
			OutputDebugStringA(DebugMsg.c_str());
			return;
		}


		Animation* animation = FindAnimation(name);
		if (animation != nullptr)
			return;

		std::wstring DebugMsg = L"Animation Name : " + name + L"\n";
		OutputDebugStringW(DebugMsg.c_str());

		animation = new Animation();
		animation->SetName(name);
		animation->CreateFrameAnimation(name, frames, baseOffset, frameOffsets, duration);
		animation->SetAnimator(this);

		Events* events = new Events();

		mAnimations.insert(std::make_pair(name, animation));
		mEvents.insert(std::make_pair(name, events));

		EventNames eventNames{};
		m_AnimationEventNames.insert(std::make_pair(name, eventNames));
	}
	

	Animation* Animator::FindAnimation(const std::wstring& name)
	{
		auto iter = mAnimations.find(name);
		if (iter == mAnimations.end())
		{
			return nullptr;
		}
		return iter->second;
	}


	void Animator::PlayAnimation(const std::wstring& name, bool loop)
	{
		Animation* animation = FindAnimation(name);

		if (animation == nullptr)
		{
			m_debugMsg = "animation is nullptr \n";
			OutputDebugStringA(m_debugMsg.c_str());
			return;
		}

		if (mActiveAnimation)
		{
			InvokeEndEvent(mActiveAnimation->GetName());
		}

		mActiveAnimation = animation;
		mActiveAnimation->Reset();
		mbLoop = loop;
		mbCompleteEventCalled = false;

		InvokeStartEvent(name);
	}

	void Animator::SetAnimationEventNames(const std::wstring& animationName, const EventNames& eventNames)
	{
		m_AnimationEventNames[animationName] = eventNames;
	}

	void Animator::RegisterEvent(const std::wstring& eventName, std::function<void()> func)
	{
		m_EventTable[eventName] = std::move(func);
	}

	void Animator::InvokeStartEvent(const std::wstring& animationName)
	{
		auto animEventIt = m_AnimationEventNames.find(animationName);

		if (animEventIt == m_AnimationEventNames.end())
			return;

		InvokeEvent(animEventIt->second.startEventName);
	}

	void Animator::InvokeCompleteEvent(const std::wstring& animationName)
	{
		auto animEventIt = m_AnimationEventNames.find(animationName);

		if (animEventIt == m_AnimationEventNames.end())
			return;

		InvokeEvent(animEventIt->second.completeEventName);
	}

	void Animator::InvokeEndEvent(const std::wstring& animationName)
	{
		auto animEventIt = m_AnimationEventNames.find(animationName);

		if (animEventIt == m_AnimationEventNames.end())
			return;

		InvokeEvent(animEventIt->second.endEventName);
	}


	void Animator::InvokeEvent(const std::wstring& eventName)
	{
		if (eventName.empty())
			return;

		auto it = m_EventTable.find(eventName);

		if (it == m_EventTable.end())
		{
			OutputDebugStringW((L"Animation Event Not Found: " + eventName + L"\n").c_str());
			return;
		}

		it->second();
	}

	Animator::Events* Animator::FindEvent(const std::wstring& name)
	{
		auto iter = mEvents.find(name);

		if (iter == mEvents.end())
		{
			return nullptr;
		}
		return iter->second;
	}

	std::function<void()>& Animator::GetStartEvent(const std::wstring& name)
	{
		Events* event = FindEvent(name);

		return event->startEvent.mEvent;
	}

	std::function<void()>& Animator::GetCompleteEvent(const std::wstring& name)
	{
		Events* event = FindEvent(name);
		return event->completeEvent.mEvent;
	}

	std::function<void()>& Animator::GetEndEvent(const std::wstring& name)
	{
		Events* event = FindEvent(name);
		return event->endEvent.mEvent;
	}
}
