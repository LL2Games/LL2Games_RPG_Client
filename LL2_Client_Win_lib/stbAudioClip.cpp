#include "stbAudioClip.h"
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

namespace
{
    int g_audioClipAliasSeed = 0;

    void LogMciError(const wchar_t* prefix, MCIERROR error)
    {
        if (error == 0)
            return;

        wchar_t errorText[256] = {};
        mciGetErrorStringW(error, errorText, 256);

        OutputDebugStringW(prefix);
        OutputDebugStringW(errorText);
        OutputDebugStringW(L"\n");
    }
}

namespace stb
{
    AudioClip::AudioClip()
        : Resource(enums::eResourceType::AudioClip)
        , mAlias(L"")
        , mOpened(false)
    {
    }

    AudioClip::~AudioClip()
    {
        Close();
    }

    HRESULT AudioClip::Load(const std::wstring& path)
    {
        Close();

        mAlias = L"AudioClip_" + std::to_wstring(++g_audioClipAliasSeed);

        std::wstring command =
            L"open \"" + path + L"\" type mpegvideo alias " + mAlias;

        MCIERROR result = mciSendStringW(command.c_str(), nullptr, 0, nullptr);
        if (result != 0)
        {
            LogMciError(L"[AudioClip] open failed: ", result);
            mAlias.clear();
            mOpened = false;
            return S_FALSE;
        }

        mOpened = true;
        return S_OK;
    }

    void AudioClip::Play(bool loop)
    {
        if (!mOpened)
            return;

        Stop();

        std::wstring command = L"play " + mAlias;
        if (loop)
            command += L" repeat";

        MCIERROR result = mciSendStringW(command.c_str(), nullptr, 0, nullptr);
        LogMciError(L"[AudioClip] play failed: ", result);
    }

    void AudioClip::Stop()
    {
        if (!mOpened)
            return;

        std::wstring stopCommand = L"stop " + mAlias;
        mciSendStringW(stopCommand.c_str(), nullptr, 0, nullptr);

        std::wstring seekCommand = L"seek " + mAlias + L" to start";
        mciSendStringW(seekCommand.c_str(), nullptr, 0, nullptr);
    }

    void AudioClip::Close()
    {
        if (!mOpened)
            return;

        std::wstring command = L"close " + mAlias;
        mciSendStringW(command.c_str(), nullptr, 0, nullptr);

        mAlias.clear();
        mOpened = false;
    }
}