#pragma once
#include "stbComponent.h"
#include "stbTexture.h"
#include "stbAnimation.h"
namespace stb
{
	class Animator : public Component 
	{
	public:
		struct Event
		{
			void operator=(std::function<void()>&& func)
			{
				mEvent = move(func);
			}

			void operator()()
			{
				if (mEvent)
				{
					mEvent();
				}
			}

			std::function<void()> mEvent;
		};

		struct EventNames
		{
			std::wstring startEventName;
			std::wstring completeEventName;
			std::wstring endEventName;
		};
		
		// ev2 = ev1;
		// ev2 = std::function<void()>(Add);

		struct Events
		{

			Event startEvent;
			Event completeEvent;
			Event endEvent;
		};

		Events* FindEvent(const std::wstring& name);
		std::function<void()>& GetStartEvent(const std::wstring& name);
		std::function<void()>& GetCompleteEvent(const std::wstring& name);
		std::function<void()>& GetEndEvent(const std::wstring& name);

		Animator();
		~Animator();

		void Initialize() override;
		void Update() override; 
		void LateUpdate() override;
		void Render(HDC hdc) override;
		void Render(stbD2DRenderer& renderer) override;

		void CreateAnimation(const std::wstring& name
			, Texture* spriteTexture
			, Vector2 leftTop
			, Vector2 size
			, Vector2 offset
			, UINT spriteLength
			, float duration);

		/**
		* Func Name: CreateAnimationByFolder
		* @brief 폴더에 저장된 이미지들로 프레임 애니메이션을 생성한다.
		*
		* @param name 애니메이션 이름
		* @param path 이미지 폴더 경로
		* @param offset 프레임 렌더링 오프셋
		* @param duration 프레임당 재생 시간
		*
		* @throws std::overflow_error
		*         생성되는 스프라이트 시트의 좌표 또는 크기가
		*         GDI에서 지원하는 int 범위를 초과하는 경우
		*/
		void CreateAnimationByFolder(const std::wstring& name
			, const std::wstring& path
			, Vector2 offset, float duration);

		void CreateFrameAnimation(const std::wstring& name
			, const std::vector<Texture*>& frames
			, Vector2 origin
			, Vector2 offset
			, float duration
		);

		void CreateFrameAnimation(const std::wstring& name
			, const std::vector<Texture*>& frames
			, Vector2 baseOffset
			, const std::vector<stb::math::Vector2>& frameOffsets
			, float duration
		);

		Animation* FindAnimation(const std::wstring& name);
		void PlayAnimation(const std::wstring& name, bool loop);


		void SetAnimationEventNames(const std::wstring& animationName, const EventNames& eventNames);
		void RegisterEvent(const std::wstring& eventName, std::function<void()> func);

		void InvokeStartEvent(const std::wstring& animationName);
		void InvokeCompleteEvent(const std::wstring& animationName);
		void InvokeEndEvent(const std::wstring& animationName);


	public:
		void SetFlipX(bool flipX) { m_flipX = flipX; }
	private: 
		void InvokeEvent(const std::wstring& eventName);
	private:
		std::map<std::wstring, Animation*> mAnimations;
		std::map<std::wstring, EventNames> m_AnimationEventNames;
		std::map<std::wstring, Event> m_EventTable;

		Animation* mActiveAnimation;
		bool mbLoop;
		bool mbCompleteEventCalled;
		bool m_flipX;
		std::map<std::wstring, Events*> mEvents;


		

		std::string m_debugMsg;
	};
}
