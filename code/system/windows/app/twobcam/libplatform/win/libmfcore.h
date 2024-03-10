#ifndef _LIBMFCORE_H_
#define _LIBMFCORE_H_

#include <map>
#include <mutex>
#include <vector>
#include <Windows.h>
#include "libmfsource.h"
#include "libcamcoreif.h"
#include "libbmp.h"
#include "libmfvideosink.h"
#include "libmfphotosink.h"
#include "libmfpreviewsink.h"
#include <queue>

namespace NSCAM {

class CamCoreHelper {
public:
    static ErrorCode OnSampleArrived(void*, DWORD, DWORD, IMFSample*, LONGLONG);
};

class CamCore : public ICamCore {
    friend CamCoreHelper;

public:
    explicit CamCore(unsigned int);
    ~CamCore(void);

    ErrorCode StartPreview              (RenderCallback)                        override;
    ErrorCode StopPreview               (void)                                  override;
    ErrorCode TakePhoto                 (const char*)                           override;
    ErrorCode StartRecord               (const char*)                           override;
    ErrorCode StopRecord                (void)                                  override;
    ErrorCode GetSupportedFormat        (PinType, MediaFormat*, unsigned int*)  override;
    ErrorCode GetCurrentFormat          (PinType, MediaFormat*)                 override;
    ErrorCode SetCurrentFormat          (PinType, unsigned long long)           override;

    typedef struct {
        enum EventType {
            START_STREAM = 0,
            STOP_STREAM,
        } Type;
        unsigned int StreamIndex;
    } Event;

    static bool                         sIsEventLooping;
    static std::mutex                   sEventMutex;
    static std::condition_variable      sCV;
    static std::queue<Event>            sEvents;

private:
    ErrorCode Initialize                (unsigned int);
    ErrorCode Release                   (void);
    ErrorCode StartStreaming            (unsigned int);
    ErrorCode StopStreaming             (unsigned int);
    void      EventLoop                 (void);

    struct RenderInfo {
        unsigned int    StreamIndex;
        void*           Caller;
        RenderCallback  Callback;
    };

    DWORD                               mPreviewPinIndex;
    DWORD                               mPhotoPinIndex;
    DWORD                               mRecordPinIndex;
    std::mutex                          mMutex;
    std::map<int, unsigned int>         mWorkingStreamRefCountMapper;
    std::vector<MediaFormat>            mPreviewSupportedFormat;
    std::vector<MediaFormat>            mRecordSupportedFormat;
    std::vector<MediaFormat>            mPhotoSupportedFormat;
    std::vector<RenderInfo>             mRenderInfoList;
    MediaSourceControl*                 mpMediaSourceControl;
    std::shared_ptr<VideoSinkDevice>    mpViderSinker;
    std::shared_ptr<PhotoSinkDevice>    mpPhotoSinker;
    std::shared_ptr<PreviewSinkDevice>  mpPreivewSinker;
    std::thread                         mEventLoopThread;
};

}

#endif