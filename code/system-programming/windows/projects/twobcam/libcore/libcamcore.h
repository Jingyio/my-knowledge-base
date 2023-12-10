#ifndef _LIBCAMCORE_H_
#define _LIBCAMCORE_H_

#ifdef LIBCAMCORE_EXPORTS
#define LIBCAMCORE_API __declspec(dllexport)
#else
#define LIBCAMCORE_API __declspec(dllimport)
#endif

#include "libifcamcore.h"
#include "../libPlatform/libifwin.h"
#include "libphotosink.h"
#include <map>
#include <mutex>
#include <vector>

namespace NSCAM {

class CamCoreHelper {
public:
    static ErrorCode OnDataArrived(void*, int, unsigned char*, MediaFormat);
};

class CamCore {
    friend CamCoreHelper;

public:
    static CamCore* CreateInstance(unsigned int);
              ~CamCore          (void);

    ErrorCode StartPreview      (RenderCallback);
    ErrorCode StopPreview       (void);
    ErrorCode TakePhoto         (const char*);
    ErrorCode StartRecord       (const char*);
    ErrorCode StopRecord        (void);
    ErrorCode GetSupportedFormat(PinType, MediaFormat*, unsigned int*);

    static std::mutex           sMutex;


private:
    explicit  CamCore           (unsigned int);
    ErrorCode Initialize        (unsigned int);
    ErrorCode Release           (void);
    ErrorCode StartStreaming    (unsigned int);
    ErrorCode StopStreaming     (unsigned int);
    ErrorCode RegisterRenderCallback(void*, unsigned int, RenderCallback);
    ErrorCode UnregisterRenderCallback(void*, unsigned int);

    struct RenderInfo {
        unsigned int StreamIndex;
        void* Caller;
        RenderCallback Callback;
    };

    PlatformHandle              mHandle;
    int                         mPreviewPinIndex;
    int                         mPhotoPinIndex;
    int                         mRecordPinIndex;
    std::mutex                  mMutex;
    std::map<int, unsigned int> mWorkingStreamRefCountMapper;
    std::vector<MediaFormat>    mPreviewSupportedFormat;
    std::vector<MediaFormat>    mRecordSupportedFormat;
    std::vector<MediaFormat>    mPhotoSupportedFormat;
    std::vector<RenderInfo>     mRenderInfoList;
    std::atomic_bool            mIsTakingPhoto;
};

}

#endif