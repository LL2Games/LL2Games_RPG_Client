#pragma once
#include "../LL2_Client_Win_Source/stbResource.h"

namespace stb
{
    class AudioClip : public Resource
    {
    public:
        AudioClip();
        ~AudioClip();

        HRESULT Load(const std::wstring& path) override;

        void Play(bool loop = false);
        void Stop();
        void Close();

    private:
        std::wstring mAlias;
        bool mOpened;
    };
}