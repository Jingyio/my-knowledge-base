#include "libmfphotosink.h"
#include "liblog.h"
#include "libformatconverter.h"

PhotoSinkDevice::PhotoSinkDevice(
    DWORD streamId
) : mStreamIndex(streamId)
{

}

HRESULT PhotoSinkDevice::TakePhoto(
    const char* pFileName,
    MediaFormat mediaFormat
)
{
    HRESULT result = S_OK;

    std::lock_guard<std::mutex> lock(mMutex);

    if (pFileName)
        mpFileName = pFileName;
    else
        mpFileName = "unnamed.bmp";

    mMediaFormat = mediaFormat;
    mIsTakingPhoto = true;

    return result;
}

HRESULT PhotoSinkDevice::OnDataCallback(
    DWORD      streamId,
    DWORD      streamFlags,
    IMFSample* pSample,
    ULONGLONG  timestamp
)
{
    HRESULT result = S_OK;

    if (!pSample || streamId != mStreamIndex)
        return S_OK;
    if (!mIsTakingPhoto)
        return S_OK;

    std::lock_guard<std::mutex> lock(mMutex);

    do {
        HRESULT res = S_OK;
        CComPtr<IMFMediaBuffer> pMFBuffer = nullptr;
        CComPtr<IMF2DBuffer> p2DBuffer = nullptr;
        BYTE* pCharBuffer = nullptr;
        std::shared_ptr<unsigned char> pDest(new unsigned char[4 * 3000 * 3000],
                                             [](unsigned char* p) { delete[] p; });
        LONG pitch = 0;

        res = pSample->GetBufferByIndex(0, &pMFBuffer);
        if (SUCCEEDED(res)) {
            res = pMFBuffer->QueryInterface(IID_PPV_ARGS(&p2DBuffer));
            if (SUCCEEDED(res) && p2DBuffer) {
                res = p2DBuffer->Lock2D(&pCharBuffer, &pitch);
                if (FAILED(res))
                    break;
            } else {
                break;
            }
        }

        if (mMediaFormat.Format == NV12) {
            NV12toRGBA(pCharBuffer,
                       mMediaFormat.Width,
                       mMediaFormat.Height,
                       mMediaFormat.Stride,
                       pDest.get());
        }
        else if (mMediaFormat.Format = YUY2) {
            YUY2toRGBA(pCharBuffer,
                       mMediaFormat.Width,
                       mMediaFormat.Height,
                       mMediaFormat.Stride,
                       pDest.get());
        }


        Bmp bmp(mpFileName);
        LoadDataFromRGBA(pDest.get(), mMediaFormat.Width, mMediaFormat.Height, bmp);
        bmp.Save((uint32_t)mMediaFormat.Width, (uint32_t)mMediaFormat.Height);
        bmp.Close();

        if (p2DBuffer)
            p2DBuffer->Unlock2D();
        
    } while (0);

    mIsTakingPhoto = false;

    return result;
}


void PhotoSinkDevice::LoadDataFromRGBA(
    unsigned char*      pBuf,
    unsigned long long  width,
    unsigned long long  height,
    Bmp&                bmp
)
{
    pBuf += width * height * 4;
    pBuf -= width * 4;

    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width; j++) {
            unsigned char r = pBuf[0];
            unsigned char g = pBuf[1];
            unsigned char b = pBuf[2];
            unsigned char a = pBuf[3];

            pBuf += 4;

            bmp << r;
            bmp << g;
            bmp << b;
        }
        pBuf -= width * 4 * 2;
    }
}

void PhotoSinkDevice::LoadDataFromYUY2(
    unsigned char*      pBuf,
    unsigned long long  width,
    unsigned long long  height,
    Bmp&                bmp
)
{

}

void PhotoSinkDevice::LoadDataFromNV12(
    unsigned char*      pBuf,
    unsigned long long  width,
    unsigned long long  height,
    Bmp&                bmp
)
{

}

unsigned int PhotoSinkDevice::GetStreamIndex(void)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mStreamIndex;
}

bool PhotoSinkDevice::IsOneShot(void)
{
    return true;
}
