#ifndef _LIBMFPHOTOSINK_H_
#define _LIBMFPHOTOSINK_H_

#include <map>
#include <mutex>
#include <vector>
#include <Windows.h>
#include "libmfsource.h"
#include "libcamcoreif.h"
#include "libbmp.h"
#include "libsinkdevice.h"

class PhotoSinkDevice : public ISinkDevice
{
public:
    explicit PhotoSinkDevice(DWORD);
    ~PhotoSinkDevice        (void) = default;

    HRESULT TakePhoto       (const char*, MediaFormat);
    HRESULT OnDataCallback  (DWORD, DWORD, IMFSample*, ULONGLONG);

    unsigned int GetStreamIndex(void) override;
    bool IsOneShot(void) override;

private:
    void LoadDataFromRGBA   (unsigned char*, unsigned long long, unsigned long long, Bmp&);
    void LoadDataFromYUY2   (unsigned char*, unsigned long long, unsigned long long, Bmp&);
    void LoadDataFromNV12   (unsigned char*, unsigned long long, unsigned long long, Bmp&);


    MediaFormat             mMediaFormat;
    const DWORD             mStreamIndex;
    std::mutex              mMutex;
    bool                    mIsTakingPhoto;
    const char*             mpFileName;
};

#endif