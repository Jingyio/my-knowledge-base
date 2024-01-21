#include "libcamcore.h"
#include "libsourcecontrol.h"
#include "../../libcore/liblog.h"
#include "../../libcore/libformatconverter.h"

namespace NSCAM {

std::mutex sMutex;
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
    } else {
        return OK;
    }
}

CamCore::CamCore(unsigned int cameraIndex)
    : mPreviewPinIndex(-1)
    , mPhotoPinIndex(-1)
    , mRecordPinIndex(-1)
    , mIsTakingPhoto(false)
{
    Initialize(cameraIndex);
}

CamCore::~CamCore(void)
{

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

    res = mpMediaSourceControl->SetPinDataCallback(this, CamCoreHelper::OnDataArrived);
    if (FAILED(res))
        return Error;

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
        mpMediaSourceControl->ClearPinDataCallback(this);
        mpMediaSourceControl->ClearPinSampleCallback(this);
    }

    return OK;
}

ErrorCode CamCore::StartPreview(RenderCallback cb)
{
    ErrorCode res = OK;

    do {
        // Register render callback
        RegisterRenderCallback(&mPreviewPinIndex, mPreviewPinIndex, cb);
        res = StartStreaming(mPreviewPinIndex);

    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCore::StopPreview(void)
{
    UnregisterRenderCallback(&mPreviewPinIndex, mPreviewPinIndex);
    return StopStreaming(mPreviewPinIndex);
}

static const char* sFileName = nullptr;
ErrorCode CamCore::TakePhoto(const char* fileName)
{
    ErrorCode ret = OK;
    sFileName = fileName;

    do {
        RegisterRenderCallback(&mPhotoPinIndex,
                               mPhotoPinIndex,
                               [](unsigned char* pBuf, MediaFormat format) -> void {
            if (!sFileName)
                sFileName = "unnamed.bmp";

            PhotoSink sink(sFileName);
            unsigned char* pDest = new unsigned char[4 * 2000 * 2000];

            if (format.Format == NV12) {
                NV12toRGBA(pBuf, format.Width, format.Height, format.Stride, pDest);
            } else if (format.Format = YUY2) {
                YUY2toRGBA(pBuf, format.Width, format.Height, format.Stride, pDest);
            }

            sink.LoadDataFromRGBA(pDest, format.Width, format.Height);
            sink.SaveFile(format.Width, format.Height);

            sFileName = nullptr;
            delete[] pDest;
        });

        mIsTakingPhoto = true;
        StartStreaming(mPhotoPinIndex);
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
        mpViderSinker = std::make_shared<VideoSinker>();
    }

    if (mpViderSinker) {
        CComPtr<IMFMediaType> pMediaType = nullptr;

        StartStreaming(mRecordPinIndex);
        std::lock_guard<std::mutex> lock(mMutex);

        result = mpMediaSourceControl->GetCurrentMediaType(mRecordPinIndex, &pMediaType);
        if (FAILED(result))
            return ErrorCode::Error;

        mpViderSinker->Start(L"test.wmv", pMediaType) == S_OK ? ErrorCode::OK : ErrorCode::Error;
        mIsRecording = true;
    }

    return ErrorCode::OK;
}

ErrorCode CamCore::StopRecord(void)
{
    HRESULT result = S_OK;

    if (!mpViderSinker)
        return ErrorCode::OK;

    result = mpViderSinker->Stop();
    if (SUCCEEDED(result)) {
        std::lock_guard<std::mutex> lock(mMutex);
        mIsRecording = false;
    }

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

ErrorCode CamCore::SetCurrentFormat(PinType pinType, unsigned int typeIndex)
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

    if (SUCCEEDED(mpMediaSourceControl->SetCurrentMediaFormat(pin, typeIndex)))
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

ErrorCode CamCore::RegisterRenderCallback(void* caller, unsigned int streamIndex, RenderCallback cb)
{
    ErrorCode res = OK;

    do {
        std::lock_guard<std::mutex> lock(mMutex);

        if (!caller || streamIndex == -1 || !cb) {
            res = INVALID_PARAM;
            break;
        }

        RenderInfo info = { 0 };
        info.Caller = caller;
        info.StreamIndex = streamIndex;
        info.Callback = cb;

        mRenderInfoList.push_back(info);

    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCore::UnregisterRenderCallback(void* caller, unsigned int streamIndex)
{
    ErrorCode res = OK;

    do {
        std::lock_guard<std::mutex> lock(mMutex);

        if (!caller || streamIndex == -1) {
            res = INVALID_PARAM;
            break;
        }

        for (auto item = mRenderInfoList.begin(); item != mRenderInfoList.end(); ) {
            if (item->Caller == caller && item->StreamIndex == streamIndex)
                item = mRenderInfoList.erase(item);
            else
                item += 1;
        }

    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCoreHelper::OnDataArrived(void* caller, int streamIndex, unsigned char* pBuf, MediaFormat format)
{
    if (!caller || !pBuf)
        return ErrorCode::INVALID_PARAM;

    CamCore* pCamCore = static_cast<CamCore*>(caller);

    for (auto& item : pCamCore->mRenderInfoList) {
        if (item.StreamIndex == streamIndex) {
            item.Callback(pBuf, format);
        }
    }

    if (streamIndex == pCamCore->mPhotoPinIndex && pCamCore->mIsTakingPhoto) {
        pCamCore->UnregisterRenderCallback(&pCamCore->mPhotoPinIndex, streamIndex);
        pCamCore->StopStreaming(pCamCore->mPhotoPinIndex);
        pCamCore->mIsTakingPhoto = false;
    }

    return OK;
}

ErrorCode CamCoreHelper::OnSampleArrived(void* caller, int streamIndex, IMFSample* pSample, LONGLONG timestamp)
{
    if (!caller || !pSample)
        return ErrorCode::INVALID_PARAM;

    CamCore* pCamCore = static_cast<CamCore*>(caller);
    if (streamIndex == pCamCore->mRecordPinIndex && pCamCore->mIsRecording) {
        pCamCore->mpViderSinker->Write(pSample, timestamp);
    }

    return ErrorCode::OK;
}
}
