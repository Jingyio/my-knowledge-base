#include "libmfcore.h"
#include "libmfsource.h"
#include "liblog.h"
#include "libformatconverter.h"
#include <thread>

namespace NSCAM {

std::mutex                  sMutex;
std::mutex                  CamCore::sEventMutex;
bool                        CamCore::sIsEventLooping = true;
std::condition_variable     CamCore::sCV;
std::queue<CamCore::Event>  CamCore::sEvents;


ICamCore* ICamCore::CreateInstance(unsigned int cameraIndex)
{
    std::lock_guard<std::mutex> lock(sMutex);
    return new CamCore(cameraIndex);
}

ErrorCode ICamCore::GetCameraCount(unsigned int* pCount)
{
    if (!pCount)
        return INVALID_PARAM;

    HRESULT result = MediaSourceControl::GetMFActivateCount(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                                                            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                            pCount);
    if (FAILED(result)) {
        *pCount = 0;
        return Error;
    }

    return OK;
}

CamCore::CamCore(unsigned int cameraIndex)
    : mPreviewPinIndex(-1)
    , mPhotoPinIndex(-1)
    , mRecordPinIndex(-1)
    , mEventLoopThread()
{
    Initialize(cameraIndex);
    mEventLoopThread = std::thread(&CamCore::EventLoop, this);
}

CamCore::~CamCore(void)
{
    if (mEventLoopThread.joinable()) {
        sIsEventLooping = false;
        CamCore::sCV.notify_all();

        mEventLoopThread.join();
    }
}

ErrorCode CamCore::Initialize(unsigned int cameraIndex)
{
    HRESULT res = S_OK;
    unsigned int pin = 0;

    std::lock_guard<std::mutex> lock(mMutex);

    mWorkingStreamRefCountMapper.clear();
    mRenderInfoList.clear();

    mpMediaSourceControl = MediaSourceControl::CreateInstance(cameraIndex);
    if (mpMediaSourceControl == nullptr)
        return INVALID_PARAM;

    res = mpMediaSourceControl->SetPinSampleCallback(this, CamCoreHelper::OnSampleArrived);
    if (FAILED(res))
        return Error;

    if (FAILED(mpMediaSourceControl->GetPreviewPinIndex(mPreviewPinIndex)))
        mPreviewPinIndex = 0;
    if (FAILED(mpMediaSourceControl->GetRecordPinIndex(mRecordPinIndex)))
        mRecordPinIndex = 0;
    if (FAILED(mpMediaSourceControl->GetPhotoPinIndex(mPhotoPinIndex)))
        mPhotoPinIndex = 0;

    mPreviewSupportedFormat.clear();
    for (int i = 0; ; i++) {
        MediaFormat format;
        res = mpMediaSourceControl->GetAvaliableMediaFormat(mPreviewPinIndex, i, format);
        if (SUCCEEDED(res))
            mPreviewSupportedFormat.push_back(format);
        else
            break;
    }

    mRecordSupportedFormat.clear();
    for (int i = 0; ; i++) {
        MediaFormat format;
        res = mpMediaSourceControl->GetAvaliableMediaFormat(mRecordPinIndex, i, format);
        if (SUCCEEDED(res))
            mRecordSupportedFormat.push_back(format);
        else
            break;
    }

    mPhotoSupportedFormat.clear();
    for (int i = 0; ; i++) {
        MediaFormat format;
        res = mpMediaSourceControl->GetAvaliableMediaFormat(mPhotoPinIndex, i, format);
        if (SUCCEEDED(res))
            mPhotoSupportedFormat.push_back(format);
        else
            break;
    }

    return OK;
}

ErrorCode CamCore::Release(void)
{
    if (mpMediaSourceControl) {
        mpMediaSourceControl->ClearPinSampleCallback(this);
    }

    return OK;
}

ErrorCode CamCore::StartPreview(RenderCallback cb)
{
    ErrorCode res = OK;
    MediaFormat format{};

    do {
        res = StartStreaming(mPreviewPinIndex);
        if (res != ErrorCode::OK)
            break;

        {
            std::lock_guard<std::mutex> lock(mMutex);
            mpPreivewSinker = nullptr;
            mpPreivewSinker = std::make_shared<PreviewSinkDevice>(mPreviewPinIndex);

            if (!mpPreivewSinker)
                break;
        }

        if (GetCurrentFormat(PHOTO, &format) == ErrorCode::OK) {
            res = SUCCEEDED(mpPreivewSinker->Start(cb, format)) ?
                            ErrorCode::OK :
                            ErrorCode::Error;
        }

    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCore::StopPreview(void)
{
    if (mpPreivewSinker)
        mpPreivewSinker->Stop();

    return StopStreaming(mPreviewPinIndex);
}

ErrorCode CamCore::TakePhoto(const char* fileName)
{
    ErrorCode ret = OK;
    MediaFormat format;

    do {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mpPhotoSinker = nullptr;
            mpPhotoSinker = std::make_shared<PhotoSinkDevice>(mPhotoPinIndex);

            if (!mpPhotoSinker)
                break;
        }

        StartStreaming(mPhotoPinIndex);

        if (GetCurrentFormat(PHOTO, &format) == ErrorCode::OK) {
            ret = SUCCEEDED(mpPhotoSinker->TakePhoto(fileName, format)) ?
                            ErrorCode::OK :
                            ErrorCode::Error;
        }

    } while (0);

    TRACE_AND_RETURN(ret);
}

ErrorCode CamCore::StartRecord(const char* pFileName)
{
    HRESULT result = S_OK;

    if (!mpMediaSourceControl)
        return ErrorCode::UNINITIALIZED;

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mpViderSinker = nullptr;
        mpViderSinker = std::make_shared<VideoSinkDevice>(mRecordPinIndex);
    }

    if (mpViderSinker) {
        CComPtr<IMFMediaType> pMediaType = nullptr;

        StartStreaming(mRecordPinIndex);
        std::lock_guard<std::mutex> lock(mMutex);

        result = mpMediaSourceControl->GetCurrentMediaType(mRecordPinIndex, &pMediaType);
        if (FAILED(result))
            return ErrorCode::Error;

        result = mpViderSinker->Start(pFileName, pMediaType);    }

    return SUCCEEDED(result) ? ErrorCode::OK : ErrorCode::Error;
}

ErrorCode CamCore::StopRecord(void)
{
    HRESULT result = S_OK;

    if (!mpViderSinker)
        return ErrorCode::OK;

    result = mpViderSinker->Stop();
    StopStreaming(mRecordPinIndex);

    return ErrorCode::OK;
}

ErrorCode CamCore::GetSupportedFormat(PinType pinType, MediaFormat* pDest, unsigned int* pCount)
{
    std::vector<MediaFormat>* pSource = nullptr;

    if (!pDest || !pCount)
        return INVALID_PARAM;

    std::lock_guard<std::mutex> lock(mMutex);

    switch (pinType) {
    case PREVIEW:
        pSource = &mPreviewSupportedFormat;
        break;
    case RECORD:
        pSource = &mRecordSupportedFormat;
        break;
    case PHOTO:
        pSource = &mPhotoSupportedFormat;
        break;
    default:
        break;
    }

    if (!pSource)
        return INVALID_PARAM;

    for (int i = 0; i < pSource->size(); i++) {
        memcpy(pDest + i, &(pSource->at(i)), sizeof(MediaFormat));
    }
    *pCount = (unsigned int)pSource->size();

    return OK;
}

ErrorCode CamCore::GetCurrentFormat(PinType pinType, MediaFormat* pFormat)
{
    DWORD pin = 0;

    if (!mpMediaSourceControl)
        return ErrorCode::UNINITIALIZED;
    if (!pFormat)
        return ErrorCode::INVALID_PARAM;

    std::lock_guard<std::mutex> lock(mMutex);
    
    switch (pinType)
    {
    case PinType::RECORD:
        pin = mRecordPinIndex;
        break;
    case PinType::PHOTO:
        pin = mPhotoPinIndex;
        break;
    case PinType::PREVIEW:
    default:
        pin = mPreviewPinIndex;
        break;
    }

    if (SUCCEEDED(mpMediaSourceControl->GetCurrentMediaFormat(pin, *pFormat)))
        return ErrorCode::OK;
    else
        return ErrorCode::Error;
}

ErrorCode CamCore::SetCurrentFormat(PinType pinType, unsigned long long typeIndex)
{
    DWORD pin = 0;

    if (!mpMediaSourceControl)
        return ErrorCode::UNINITIALIZED;

    std::lock_guard<std::mutex> lock(mMutex);

    switch (pinType)
    {
    case PinType::RECORD:
        pin = mRecordPinIndex;
        break;
    case PinType::PHOTO:
        pin = mPhotoPinIndex;
        break;
    case PinType::PREVIEW:
    default:
        pin = mPreviewPinIndex;
        break;
    }

    if (mWorkingStreamRefCountMapper.count(pin) != 0)
        return ErrorCode::INVALID_REQUEST;

    if (SUCCEEDED(mpMediaSourceControl->SetCurrentMediaFormat(pin, (DWORD)typeIndex)))
        return ErrorCode::OK;
    else
        return ErrorCode::Error;

}

ErrorCode CamCore::StartStreaming(unsigned int streamIndex)
{
    HRESULT res = S_OK;
    ErrorCode ret = OK;

    if (!mpMediaSourceControl)
        return UNINITIALIZED;

    do {
        std::lock_guard<std::mutex> lock(mMutex);

        auto item = mWorkingStreamRefCountMapper.find(streamIndex);
        if (item != mWorkingStreamRefCountMapper.end()) {       // Working
            item->second += 1;
            break;
        } else {
            res = mpMediaSourceControl->SetStreamState(streamIndex, true);
            if (FAILED(res)) {
                ret = Error;
                break;
            }

            mWorkingStreamRefCountMapper[streamIndex] = 1;
        }

    } while (0);

    TRACE_AND_RETURN(ret);
}

ErrorCode CamCore::StopStreaming(unsigned int streamIndex)
{
    ErrorCode ret = OK;
    HRESULT res = S_OK;

    if (!mpMediaSourceControl)
        return UNINITIALIZED;

    do {
        std::lock_guard<std::mutex> lock(mMutex);

        auto item = mWorkingStreamRefCountMapper.find(streamIndex);
        if (item != mWorkingStreamRefCountMapper.end()) {
            item->second -= 1;
        } else {
            break;
        }

        if (item->second <= 0) {
            mWorkingStreamRefCountMapper.erase(item);

            int retryCount = 0;
            do {
                BOOL isRunning = FALSE;
                
                res = mpMediaSourceControl->GetStreamState(streamIndex, isRunning);
                if (FAILED(res) || isRunning == FALSE)
                    break;

                res = mpMediaSourceControl->SetStreamState(streamIndex, false);
                if (FAILED(res)) {
                    retryCount++;
                    Log::GetInstance().Fail("retry %d times\n", retryCount);
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

            } while (FAILED(res));
        }

    } while (0);

    TRACE_AND_RETURN(ret);
}

void CamCore::EventLoop(void)
{
    while (CamCore::sIsEventLooping) {
        std::unique_lock<std::mutex> lock(CamCore::sEventMutex);
        sCV.wait(lock, []() { return !sEvents.empty() || !CamCore::sIsEventLooping; });

        while (!sEvents.empty()) {
            auto event = sEvents.front();
            sEvents.pop();

            if (event.Type == CamCore::Event::EventType::START_STREAM) {
                StartStreaming(event.StreamIndex);
            } else {
                StopStreaming(event.StreamIndex);
            }
        }
    }
}

ErrorCode CamCoreHelper::OnSampleArrived(
    void* caller,
    DWORD streamIndex,
    DWORD streamFlags,
    IMFSample* pSample,
    LONGLONG timestamp
)
{
    if (!caller || !pSample)
        return ErrorCode::INVALID_PARAM;

    CamCore* pCamCore = static_cast<CamCore*>(caller);

    if (pCamCore->mpPreivewSinker)
        pCamCore->mpPreivewSinker->OnDataCallback(streamIndex, streamFlags, pSample, timestamp);

    if (pCamCore->mpPhotoSinker) {
        pCamCore->mpPhotoSinker->OnDataCallback(streamIndex, streamFlags, pSample, timestamp);

        if (pCamCore->mpPhotoSinker->IsOneShot()) {
            CamCore::Event event = {
                .Type = CamCore::Event::EventType::STOP_STREAM,
                .StreamIndex = pCamCore->mpPhotoSinker->GetStreamIndex(),
            };
            CamCore::sEvents.push(event);
            CamCore::sCV.notify_one();

            pCamCore->mpPhotoSinker = nullptr;
        }
    }

    if (pCamCore->mpViderSinker)
        pCamCore->mpViderSinker->OnDataCallback(streamIndex, streamFlags, pSample, timestamp);

    return ErrorCode::OK;
}
}
