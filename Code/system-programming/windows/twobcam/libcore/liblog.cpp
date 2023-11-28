#include "liblog.h"
#include <cstdarg>
#include <format>
#include <Windows.h>
#include <iostream>

namespace NSCAM {

const char* const Log::sFilePath = "camera.log";

Log& Log::GetInstance(void)
{
    static Log instance;
    return instance;
}

Log::Log(void)
    : mCacheString()
    , mMutex()
    , mCV()
    , mIsRunning(false)
{
    mCacheString.clear();
    mIsRunning = true;

    std::thread([=]() {
        while (mIsRunning) {
            std::unique_lock<std::mutex> lock(mMutex);
            mCV.wait_for(lock, std::chrono::milliseconds(1000), [this] { return !mIsRunning; });
            if (mIsRunning) {
                Dump(mCacheString);
                mCacheString.clear();
            }
        }
    }).detach();
}

Log::~Log(void)
{
    mIsRunning = false;
    mCV.notify_all();
}

void Log::Dump(const std::string& str)
{
    std::ofstream fileStream;
    fileStream.open(sFilePath, std::ios::out | std::ios::app);
    fileStream << str;
    fileStream.close();
}

void Log::Trace(const char* format, ...)
{
    char buf[512] = { 0 };
    std::string prefix = std::format("[P{:05d}:T{:05d}] [Trace  ] ",
        GetCurrentProcessId(),
        GetCurrentThreadId());

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    mCacheString += prefix;
    mCacheString += std::string(buf);
    mCacheString += "\n";
}

void Log::Fail(const char* format, ...)
{
    char buf[512] = { 0 };
    std::string prefix = std::format("[P{:05d}:T{:05d}] [Fail   ] ",
        GetCurrentProcessId(),
        GetCurrentThreadId());

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    mCacheString += prefix;
    mCacheString += std::string(buf);
    mCacheString += "\n";
}

void Log::Succeed(const char* format, ...)
{
    char buf[512] = { 0 };
    std::string prefix = std::format("[P{:05d}:T{:05d}] [Succeed] ",
        GetCurrentProcessId(),
        GetCurrentThreadId());

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    mCacheString += prefix;
    mCacheString += std::string(buf);
    mCacheString += "\n";
}

void Log::Enter(const char* format, ...)
{
    char buf[512] = { 0 };
    std::string prefix = std::format("[P{:05d}:T{:05d}] [Enter  ] ",
        GetCurrentProcessId(),
        GetCurrentThreadId());

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    mCacheString += prefix;
    mCacheString += std::string(buf);
    mCacheString += "\n";
}

void Log::Leave(const char* format, ...)
{
    char buf[512] = { 0 };
    std::string prefix = std::format("[P{:05d}:T{:05d}] [Leave  ] ",
        GetCurrentProcessId(),
        GetCurrentThreadId());

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    mCacheString += prefix;
    mCacheString += std::string(buf);
    mCacheString += "\n";
}
}