#include "libcamvideosink.h"
#include "../../libcore/liblog.h"
#include "../../libcore/libformatconverter.h"

HRESULT VideoSinker::Start(LPCWSTR fileName, IMFMediaType* pMediaType)
{
    HRESULT result = S_OK;
    LPCWSTR pName = nullptr;

    if (fileName)
        pName = fileName;
    else
        pName = L"record.wmv";

    std::lock_guard<std::mutex> lock(mMutex);

    mpSinkWriter = nullptr;
    mStreamIndex = 0;
    mBaseTimestamp = 0;
    mIsFirstSample = true;

    do {
        result = MFCreateSinkWriterFromURL(pName, nullptr, nullptr, &mpSinkWriter);
        if (FAILED(result))
            break;

        result = mpSinkWriter->AddStream(pMediaType, &mStreamIndex);
        if (FAILED(result))
            break;

        result = mpSinkWriter->SetInputMediaType(mStreamIndex, pMediaType, nullptr);
        if (FAILED(result))
            break;

        result = mpSinkWriter->BeginWriting();

    } while (0);

    return result;
}

HRESULT VideoSinker::Write(IMFSample* pSample, LONGLONG timestamp)
{
    HRESULT result = S_OK;

    if (!pSample)
        return S_OK;

    std::lock_guard<std::mutex> lock(mMutex);
    if (mIsFirstSample) {
        mIsFirstSample = false;
        mBaseTimestamp = timestamp;
    }

    timestamp = timestamp - mBaseTimestamp;
    pSample->SetSampleTime(timestamp);
    if (mpSinkWriter) {
        result = mpSinkWriter->WriteSample(mStreamIndex, pSample);
    }

    return result;
}

HRESULT VideoSinker::Stop(void)
{
    std::lock_guard<std::mutex> lock(mMutex);
    if (mpSinkWriter)
        return mpSinkWriter->Finalize();
    else
        return S_OK;
}