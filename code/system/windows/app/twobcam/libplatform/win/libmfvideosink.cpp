#include "libmfvideosink.h"
#include "liblog.h"
#include "libformatconverter.h"

VideoSinkDevice::VideoSinkDevice(
    DWORD streamId
)   : mStreamIndex(streamId)
{

}

HRESULT VideoSinkDevice::Start(
    const char*   pFileName,
    IMFMediaType* pMediaType
)
{
    HRESULT result = S_OK;
    int length = MultiByteToWideChar(CP_UTF8, 0, pFileName, -1, NULL, 0);
    std::shared_ptr<wchar_t> pWideString(new wchar_t[length],
                                         [](wchar_t* p) {delete[]p; });

    MultiByteToWideChar(CP_UTF8, 0, pFileName, -1, pWideString.get(), length);

    std::lock_guard<std::mutex> lock(mMutex);

    mpSinkWriter   = nullptr;
    mChannelIndex  = 0;
    mBaseTimestamp = 0;
    mIsFirstSample = true;

    do {
        result = MFCreateSinkWriterFromURL(pWideString.get(),
                                           nullptr,
                                           nullptr,
                                           &mpSinkWriter);
        if (FAILED(result))
            break;

        result = mpSinkWriter->AddStream(pMediaType, &mChannelIndex);
        if (FAILED(result))
            break;

        result = mpSinkWriter->SetInputMediaType(mChannelIndex,
                                                 pMediaType,
                                                 nullptr);
        if (FAILED(result))
            break;

        result = mpSinkWriter->BeginWriting();

    } while (0);

    return result;
}

HRESULT VideoSinkDevice::Stop(
    void
)
{
    //std::lock_guard<std::mutex> lock(mMutex);

    if (mpSinkWriter)
        return mpSinkWriter->Finalize();
    else
        return S_OK;
}

HRESULT VideoSinkDevice::OnDataCallback(
    DWORD      streamId,
    DWORD      streamFlags,
    IMFSample* pSample,
    ULONGLONG  timestamp
)
{
    HRESULT result = S_OK;

    if (!pSample || streamId != mStreamIndex)
        return S_OK;

    std::lock_guard<std::mutex> lock(mMutex);
    if (mIsFirstSample) {
        mIsFirstSample = false;
        mBaseTimestamp = timestamp;
    }

    timestamp = timestamp - mBaseTimestamp;
    pSample->SetSampleTime(timestamp);
    if (mpSinkWriter)
        result = mpSinkWriter->WriteSample(mChannelIndex, pSample);

    return result;
}

unsigned int VideoSinkDevice::GetStreamIndex(void)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mStreamIndex;
}

bool VideoSinkDevice::IsOneShot(void)
{
    return false;
}
