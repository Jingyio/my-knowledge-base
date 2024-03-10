#ifndef _LIBMFVIDEOSINK_H_
#define _LIBMFVIDEOSINK_H_

#include <map>
#include <mutex>
#include <vector>
#include <Windows.h>
#include "libmfsource.h"
#include "libcamcoreif.h"
#include "libbmp.h"
#include "libsinkdevice.h"

class VideoSinkDevice : public ISinkDevice
{
public:
    explicit VideoSinkDevice(DWORD);
    ~VideoSinkDevice        (void) = default;

    HRESULT Start           (const char*, IMFMediaType*);
    HRESULT Stop            (void);
    HRESULT OnDataCallback  (DWORD, DWORD, IMFSample*, ULONGLONG);
    unsigned int GetStreamIndex(void) override;
    bool IsOneShot(void) override;

private:
    CComPtr<IMFSinkWriter>  mpSinkWriter;
    const DWORD             mStreamIndex;
    DWORD                   mChannelIndex;
    std::mutex              mMutex;
    LONGLONG                mBaseTimestamp;
    BOOL                    mIsFirstSample;
};

#endif