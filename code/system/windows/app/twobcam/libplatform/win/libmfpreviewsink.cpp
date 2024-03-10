#include "libmfpreviewsink.h"
#include "liblog.h"
#include "libformatconverter.h"

PreviewSinkDevice::PreviewSinkDevice(
    DWORD streamId
)   : mStreamIndex(streamId)
{

}

HRESULT PreviewSinkDevice::Start(
    RenderCallback cb,
    MediaFormat    format
)
{
    if (!cb)
        return E_INVALIDARG;

    std::lock_guard<std::mutex> lock(mMutex);

    mCB = cb;
    mFormat = format;
    mIsPreviewing = true;

    return S_OK;
}

HRESULT PreviewSinkDevice::Stop(
    void
)
{
    std::lock_guard<std::mutex> lock(mMutex);

    mIsPreviewing = false;
    mCB = nullptr;

    return S_OK;
}

HRESULT PreviewSinkDevice::OnDataCallback(
    DWORD      streamId,
    DWORD      streamFlags,
    IMFSample* pSample,
    ULONGLONG  timestamp
)
{
	HRESULT res = S_OK;

    if (!pSample)
        return E_POINTER;
    if (streamId != mStreamIndex || !mIsPreviewing)
        return S_OK;

	do {
		CComPtr<IMFMediaBuffer> pMFBuffer = nullptr;
		CComPtr<IMF2DBuffer> p2DBuffer = nullptr;
		BYTE* pCharBuffer = nullptr;
        std::shared_ptr<unsigned char> pDest(new unsigned char[4 * 2000 * 2000], [](unsigned char* p) { delete[] p; });
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

        mFormat.Stride = pitch;
        if (mFormat.Format == NV12) {
            NV12toRGBA(pCharBuffer, mFormat.Width, mFormat.Height, pitch, pDest.get());
        } else if (mFormat.Format = YUY2) {
            YUY2toRGBA(pCharBuffer, mFormat.Width, mFormat.Height, pitch, pDest.get());
        }

		if (p2DBuffer)
			p2DBuffer->Unlock2D();

        if (mCB)
            mCB(pDest.get(), mFormat);

	} while (0);

	return res;
}

unsigned int PreviewSinkDevice::GetStreamIndex(void)
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mStreamIndex;
}

bool PreviewSinkDevice::IsOneShot(void)
{
    return false;
}
