#ifndef _LIBCAMCORE_H_
#define _LIBCAMCORE_H_

#include <map>
#include <mutex>
#include <vector>
#include <Windows.h>
#include "libsourcecontrol.h"
#include "../../libcore/libcamcoreif.h"
#include "../../libcore/libphotosink.h"

namespace NSCAM {

class CamCoreHelper {
public:
    static ErrorCode OnDataArrived(void*, int, unsigned char*, MediaFormat);
};

class CamCore : public ICamCore {
    friend CamCoreHelper;

public:
    explicit CamCore(unsigned int);
    ~CamCore(void);

    ErrorCode StartPreview      (RenderCallback) override;
    ErrorCode StopPreview       (void) override;
    ErrorCode TakePhoto         (const char*) override;
    ErrorCode StartRecord       (const char*) override;
    ErrorCode StopRecord        (void) override;
    ErrorCode GetSupportedFormat(PinType, MediaFormat*, unsigned int*) override;

    static std::mutex           sMutex;


private:
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

    DWORD                       mPreviewPinIndex;
    DWORD                       mPhotoPinIndex;
    DWORD                       mRecordPinIndex;
    std::mutex                  mMutex;
    std::map<int, unsigned int> mWorkingStreamRefCountMapper;
    std::vector<MediaFormat>    mPreviewSupportedFormat;
    std::vector<MediaFormat>    mRecordSupportedFormat;
    std::vector<MediaFormat>    mPhotoSupportedFormat;
    std::vector<RenderInfo>     mRenderInfoList;
    std::atomic_bool            mIsTakingPhoto;
    MediaSourceControl*         mpMediaSourceControl;
};

}

#endif