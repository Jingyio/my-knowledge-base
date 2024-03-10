#ifndef _LIBMFPREVIEWSINK_H_
#define _LIBMFPREVIEWSINK_H_

#include <map>
#include <mutex>
#include <vector>
#include <Windows.h>
#include "libmfsource.h"
#include "libcamcoreif.h"
#include "libbmp.h"
#include "libsinkdevice.h"

class PreviewSinkDevice : public ISinkDevice
{
public:
    explicit PreviewSinkDevice(DWORD);
    ~PreviewSinkDevice(void) = default;

    HRESULT Start           (RenderCallback, MediaFormat);
    HRESULT Stop            (void);
    HRESULT OnDataCallback  (DWORD, DWORD, IMFSample*, ULONGLONG) override;
    
    unsigned int GetStreamIndex(void) override;
    bool IsOneShot(void) override;

private:
    const DWORD             mStreamIndex;
    std::mutex              mMutex;
    MediaFormat             mFormat;
    RenderCallback          mCB;
    bool                    mIsPreviewing;
};

#endif