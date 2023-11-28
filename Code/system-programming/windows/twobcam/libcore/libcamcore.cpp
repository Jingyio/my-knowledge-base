#include "libcamcore.h"
#include "liblog.h"
#include "libformatconverter.h"

namespace NSCAM {

std::mutex CamCore::sMutex;

CamCore* CamCore::CreateInstance(unsigned int cameraIndex)
{
    std::lock_guard<std::mutex> lock(sMutex);
    return new CamCore(cameraIndex);
}

CamCore::CamCore(unsigned int cameraIndex)
    : mHandle()
    , mPreviewPinIndex(-1)
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
    ErrorCode res = OK;
    unsigned int pin = 0;

    std::lock_guard<std::mutex> lock(mMutex);

    mWorkingStreamRefCountMapper.clear();
    mRenderInfoList.clear();

    res = WinOpenCamera(cameraIndex, &mHandle);
    if (res)
        mHandle.Unused = nullptr;

    res = WinRegisterCameraDataCallback(mHandle, this, CamCoreHelper::OnDataArrived);
    if (res)
        return res;

    mPreviewPinIndex = 0;
    mRecordPinIndex = 0;
    mPhotoPinIndex = 0;

    if (WinGetCameraPreviewPinIndex(mHandle, pin) == 0)
        mPreviewPinIndex = pin;

    if (WinGetCameraRecordPinIndex(mHandle, pin) == 0)
        mRecordPinIndex = pin;

    if (WinGetCameraPhotoPinIndex(mHandle, pin) == 0)
        mPhotoPinIndex = pin;

    mPreviewSupportedFormat.clear();
    for (int i = 0; ; i++) {
        MediaFormat format;
        res = WinGetCameraSupportedMediaFormat(mHandle, mPreviewPinIndex, i, format);
        if (res == OK)
            mPreviewSupportedFormat.push_back(format);
        else
            break;
    }

    mRecordSupportedFormat.clear();
    for (int i = 0; ; i++) {
        MediaFormat format;
        res = WinGetCameraSupportedMediaFormat(mHandle, mRecordPinIndex, i, format);
        if (res == OK)
            mRecordSupportedFormat.push_back(format);
        else
            break;
    }

    mPhotoSupportedFormat.clear();
    for (int i = 0; ; i++) {
        MediaFormat format;
        res = WinGetCameraSupportedMediaFormat(mHandle, mPhotoPinIndex, i, format);
        if (res == OK)
            mPhotoSupportedFormat.push_back(format);
        else
            break;
    }

    return OK;
}

ErrorCode CamCore::Release(void)
{
    WinClearCameraDataCallback(mHandle, this);

    return OK;
}

ErrorCode CamCore::StartPreview(RenderCallback cb)
{
    ErrorCode res = OK;

    do {
        // Register render callback
        RegisterRenderCallback(mHandle.Unused, mPreviewPinIndex, cb);
        res = StartStreaming(mPreviewPinIndex);

    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCore::StopPreview(void)
{
    UnregisterRenderCallback(mHandle.Unused, mPreviewPinIndex);
    return StopStreaming(mPreviewPinIndex);
}

ErrorCode CamCore::TakePhoto(const char* fileName)
{
    ErrorCode res = OK;

    do {
        RegisterRenderCallback(this, mPhotoPinIndex, [](unsigned char* pBuf, MediaFormat format) -> void {
            PhotoSink sink("1.bmp");
            unsigned char* pDest = new unsigned char[4 * 1024 * 1024];

            if (format.Format == NV12) {
                NV12toRGBA(pBuf, format.Width, format.Height, format.Stride, pDest);
            } else if (format.Format = YUY2) {
                YUY2toRGBA(pBuf, format.Width, format.Height, format.Stride, pDest);
            }

            sink.LoadDataFromRGBA(pDest, format.Width, format.Height);
            sink.SaveFile(format.Width, format.Height);

            delete[] pDest;
        });

        mIsTakingPhoto = true;
        StartStreaming(mPhotoPinIndex);
    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCore::StartRecord(const char*)
{
    return OK;
}

ErrorCode CamCore::StopRecord(void)
{
    return OK;
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
    *pCount = pSource->size();

    return OK;
}

ErrorCode CamCore::StartStreaming(unsigned int streamIndex)
{
    ErrorCode res = OK;

    do {
        std::lock_guard<std::mutex> lock(mMutex);

        if (!mHandle.Unused) {
            res = INVALID_HANDLE;
            break;
        }

        if (streamIndex == -1) {
            res = INVALID_PARAM;
            break;
        }

        auto item = mWorkingStreamRefCountMapper.find(streamIndex);
        if (item != mWorkingStreamRefCountMapper.end()) {       // Working
            item->second += 1;
            break;
        } else {
            res = WinSetCameraState(mHandle, streamIndex, true);   // Start to work
            if (res)
                break;

            mWorkingStreamRefCountMapper[streamIndex] = 1;
        }

    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CamCore::StopStreaming(unsigned int streamIndex)
{
    ErrorCode res = OK;

    do {
        std::lock_guard<std::mutex> lock(mMutex);

        if (!mHandle.Unused) {
            res = INVALID_HANDLE;
            break;
        }

        if (streamIndex == -1) {
            res = INVALID_PARAM;
            break;
        }

        auto item = mWorkingStreamRefCountMapper.find(streamIndex);
        if (item != mWorkingStreamRefCountMapper.end()) {
            item->second -= 1;
        } else {
            break;
        }

        if (item->second <= 0) {
            mWorkingStreamRefCountMapper.erase(item);

            res = WinSetCameraState(mHandle, streamIndex, false);
            if (res)
                break;
        }

    } while (0);

    TRACE_AND_RETURN(res);
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
        return INVALID_PARAM;

    CamCore* pCamCore = static_cast<CamCore*>(caller);

    for (auto& item : pCamCore->mRenderInfoList) {
        if (item.StreamIndex == streamIndex) {
            item.Callback(pBuf, format);
        }
    }

    if (streamIndex == pCamCore->mPhotoPinIndex && pCamCore->mIsTakingPhoto) {
        pCamCore->StopStreaming(pCamCore->mPhotoPinIndex);
        pCamCore->mIsTakingPhoto = false;
    }

    return OK;
}
}
