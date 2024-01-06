#ifndef _LIBLOG_H_
#define _LIBLOG_H_

#include <string>
#include <fstream>
#include <mutex>

namespace NSCAM {

class Log {
public:
    static Log& GetInstance(void);
    ~Log(void);

    void Trace  (const char* format, ...);
    void Fail   (const char* format, ...);
    void Succeed(const char* format, ...);
    void Enter  (const char* format, ...);
    void Leave  (const char* format, ...);

private:
    Log(void);
    Log(const Log&) = default;
    Log& operator=(const Log&) = default;

    void Dump(const std::string& str);

    static const char* const sFilePath;
    std::string mCacheString;
    std::mutex mMutex;
    std::condition_variable mCV;
    bool mIsRunning;
};

}

#define TRACE_AND_RETURN(res)                                                                  \
    do {                                                                                       \
        if (res) {                                                                             \
            NSCAM::Log::GetInstance().Fail("%s returned with res = 0x%x", __FUNCTION__, res);  \
            return Error;                                                                      \
        } else {                                                                               \
            return OK;                                                                         \
        }                                                                                      \
    } while (0)

#endif