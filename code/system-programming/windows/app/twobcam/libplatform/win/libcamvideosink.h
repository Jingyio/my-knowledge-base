#ifndef _LIBCAMVIDEOSINK_H_
#define _LIBCAMVIDEOSINK_H_

#include <map>
#include <mutex>
#include <vector>
#include <Windows.h>
#include "libsourcecontrol.h"
#include "../../libcore/libcamcoreif.h"
#include "../../libcore/libphotosink.h"

class VideoSinker
{
public:
    explicit VideoSinker(void) = default;
    ~VideoSinker(void) = default;

    HRESULT Start(LPCWSTR, IMFMediaType*);
    HRESULT Write(IMFSample*, LONGLONG);
    HRESULT Stop(void);

private:
    CComPtr<IMFSinkWriter> mpSinkWriter;
    DWORD mStreamIndex;
    std::mutex mMutex;
    LONGLONG mBaseTimestamp;
    BOOL mIsFirstSample;
};

#endif