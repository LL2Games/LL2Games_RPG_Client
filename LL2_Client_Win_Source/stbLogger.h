#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <mutex>
#include "stbNetworkConfig.h"

//LogFormat
#include <cstdarg>
#include <cstdio>
#include <Windows.h>

namespace stb
{
    class Logger
    {
    private:
        static std::ofstream& GetLogFile()
        {
            static std::ofstream logFile;
            return logFile;
        }

        static std::mutex& GetMutex()
        {
            static std::mutex logMutex;
            return logMutex;
        }

        static bool& GetInitialized()
        {
            static bool initialized = false;
            return initialized;
        }

    public:
        static void Init()
        {
            if (!GetInitialized())
            {
                std::string charId = NetworkConfig::GetCharacterId();
                std::string filename = "client_" + charId + ".log";
                GetLogFile().open(filename, std::ios::out | std::ios::trunc);
                GetInitialized() = true;
            }
        }

        static void Log(const std::string& msg)
        {
            std::lock_guard<std::mutex> lock(GetMutex());
            if (GetLogFile().is_open())
            {
                GetLogFile() << msg;
                GetLogFile().flush();
            }
            OutputDebugStringA(msg.c_str());
        }
        
        //__FILE__이 너무 길게 나오는경우때문에 파일명 하나만 보기위해 만듦
        //기존: C:\Users\user\source\repos\LL2Games\test\LL2_Client_Win_Source\stbAnimation.cpp
        //적용: stbAnimation.cpp
        static const char* BaseFileName(const char* path)
        {
            if (path == nullptr)
                return "";

            const char* file = path;

            for (const char* p = path; *p != '\0'; ++p)
            {
                if (*p == '\\' || *p == '/')
                {
                    file = p + 1;
                }
            }

            return file;
        }

        static void LogFormat(const char* fmt, ...)
        {
#ifdef __DEV_LOG
            if (fmt == nullptr)
                return;

            va_list args;
            va_start(args, fmt);

            va_list argsCopy;
            va_copy(argsCopy, args);

            int len = std::vsnprintf(nullptr, 0, fmt, argsCopy);
            va_end(argsCopy);

            if (len < 0)
            {
                va_end(args);
                return;
            }

            std::string msg;
            msg.resize(len + 1);

            std::vsnprintf(msg.data(), msg.size(), fmt, args);
            va_end(args);

            msg.resize(len);

            // 자동 줄바꿈 추가
            msg += '\n';

            Log(msg);
#endif
        }
     

        static void Close()
        {
            if (GetLogFile().is_open())
            {
                GetLogFile().close();
            }
        }
    };

    #define LOG(msg) do { \
        std::stringstream ss; \
        ss << msg; \
        stb::Logger::Log(ss.str()); \
    } while(0)

#define M_LOGGER(fmt, ...) \
    do { \
        stb::Logger::LogFormat("[%s %s(%d)]" fmt, stb::Logger::BaseFileName(__FILE__), __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } while (0)
}
