#include "libsourcecontrol.h"
#include <ks.h>
#include <ksmedia.h>
#include <mftransform.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

std::mutex MediaSourceControl::sInstLock;
IMFActivate** MediaSourceControl::sMFActivates;

MediaSourceControl* MediaSourceControl::CreateInstance(
	unsigned int cameraIndex
)
{
	HRESULT result = S_OK;
	UINT32 count = 0;
	std::lock_guard<std::mutex> lock(sInstLock);

	if (!sMFActivates) {
		result = GetMFActivateCount(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
									MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
									&count);
		if (FAILED(result) || (cameraIndex >= count))
			return nullptr;

		sMFActivates = new IMFActivate * [128];
		memset(sMFActivates, 0, 128 * sizeof(IMFActivate*));

		for (int i = 0; ; i++) {
			result = MediaSourceControl::GetMFActivate(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
												       MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
												       i,
												       &sMFActivates[i]);
			if (FAILED(result))
				break;
		}
	}

	return new MediaSourceControl(sMFActivates[cameraIndex]);
}

MediaSourceControl::MediaSourceControl(
	IMFActivate* pDevice
)
	: mpDevice(pDevice)
	, mpMediaSource(nullptr)
	, mMutex()
	, mRefCount(1)
	, mDefaultPreviewPin(-1)
	, mDefaultRecordPin(-1)
	, mDefaultPhotoPin(-1)
{
	HRESULT res = S_OK;
	CComPtr<IMFAttributes> pAttributes = nullptr;

	res = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (SUCCEEDED(res))
		MFStartup(MF_VERSION);

	{
		std::unique_lock<std::shared_mutex> wlock(mMutex);
		mPinTypeMap.clear();
		mPinDataCallbackList.clear();

		if (mpDevice) {
			mpDevice->ActivateObject(IID_PPV_ARGS(&mpMediaSource));

			res = MFCreateAttributes(&pAttributes, 2);
			if (SUCCEEDED(res) && mpMediaSource) {
				if (SUCCEEDED(pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, TRUE)) &&
					SUCCEEDED(pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this))) {
					MFCreateSourceReaderFromMediaSource(mpMediaSource, pAttributes, &mpSourceReader);

					// Get all stream type
					CComQIPtr<IMFMediaSourceEx> pMediaSourceEx(mpMediaSource);
					if (pMediaSourceEx) {
						for (int i = 0; ; i++) {
							CComPtr<IMFAttributes> pStreamAttributes = nullptr;
							res = pMediaSourceEx->GetStreamAttributes(i, &pStreamAttributes);
							if (FAILED(res) || !pStreamAttributes)
								break;

							GUID category = GUID_NULL;
							res = pStreamAttributes->GetGUID(MF_DEVICESTREAM_STREAM_CATEGORY, &category);
							if (FAILED(res))
								continue;

							if (IsEqualGUID(category, PINNAME_PREVIEW)) {
								mPinTypeMap[i] = PinType::PREVIEW;
								if (mDefaultPreviewPin == -1)
									mDefaultPreviewPin = i;
							}
							else if (IsEqualGUID(category, PINNAME_CAPTURE)) {
								mPinTypeMap[i] = PinType::RECORD;
								if (mDefaultRecordPin == -1)
									mDefaultRecordPin = i;
							}
							else if (IsEqualGUID(category, PINNAME_IMAGE)) {
								mPinTypeMap[i] = PinType::IMAGE;
								if (mDefaultPhotoPin == -1)
									mDefaultPhotoPin = i;
							}
						}

						if ((mDefaultPhotoPin != -1) || (mDefaultRecordPin != -1) || (mDefaultPreviewPin != -1)) {
							mDefaultPhotoPin = mDefaultPhotoPin != -1 ? mDefaultPhotoPin :
											   (mDefaultRecordPin != -1 ? mDefaultRecordPin : mDefaultPreviewPin);
							mDefaultRecordPin = mDefaultRecordPin != -1 ? mDefaultRecordPin :
											   (mDefaultPreviewPin != -1 ? mDefaultPreviewPin : mDefaultPhotoPin);
							mDefaultPreviewPin = mDefaultPreviewPin != -1 ? mDefaultPreviewPin :
											     (mDefaultPhotoPin != -1 ? mDefaultPhotoPin : mDefaultRecordPin);
						}
					}

				}
			}
		}

	}

	// Shut off all streams
	SetStreamState(MF_SOURCE_READER_ALL_STREAMS, false);
}

#pragma region IUnknown
//
// IUnknown interfaces
//
HRESULT MediaSourceControl::QueryInterface(
	REFIID riid,
	void** ppvObject
)
{
	static const QITAB qit[] =
	{
		QITABENT(MediaSourceControl, IMFSourceReaderCallback),
		{ 0 },
	};

	return QISearch(this, qit, riid, ppvObject);
}

ULONG MediaSourceControl::AddRef(
	void
)
{
	return InterlockedIncrement(&mRefCount);
}

ULONG MediaSourceControl::Release(
	void
)
{
	ULONG count = InterlockedDecrement(&mRefCount);
	if (count == 0)
		delete this;

	return count;
}
#pragma endregion

#pragma region IMFSourceReaderCallback
//
// IMFSourceReaderCallback interfaces
//
HRESULT MediaSourceControl::OnReadSample(
	HRESULT result,
	DWORD streamIndex,
	DWORD streamFlags,
	LONGLONG timestamp,
	IMFSample* pSample
)
{
	HRESULT res = S_OK;
	std::unique_lock<std::shared_mutex> wlock(mMutex);

	do {
		if (SUCCEEDED(result) && pSample) {

			CComPtr<IMFMediaBuffer> pMFBuffer = nullptr;
			CComPtr<IMF2DBuffer> p2DBuffer = nullptr;
			BYTE* pCharBuffer = nullptr;
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

			// Process the IMFSample here
			for (auto& item : mPinDataCallbackList) {
				if (item.first == nullptr)
					continue;

				if (item.second) {
					MediaFormat format = mPinFormatMap[streamIndex];
					format.Stride = pitch;

					item.second(item.first, streamIndex, pCharBuffer, format);
				}
			}

			if (p2DBuffer)
				p2DBuffer->Unlock2D();
		}

		if (SUCCEEDED(result) && pSample) {
			for (auto& item : mPinSampleCallbackList) {
				if (item.first == nullptr)
					continue;

				if (item.second) {
					item.second(item.first, streamIndex, pSample, timestamp);
				}
			}
		}

	} while (0);

	if (SUCCEEDED(result) && mpSourceReader) {
		if (mWorkingPins.count(streamIndex) != 0)
			res = mpSourceReader->ReadSample(streamIndex, 0, NULL, NULL, NULL, NULL);
	}

	return S_OK;
}

HRESULT MediaSourceControl::OnFlush(
	DWORD streamIndex
)
{
	return S_OK;
}

HRESULT MediaSourceControl::OnEvent(
	DWORD streamIndex,
	IMFMediaEvent* pEvent
)
{
	return S_OK;
}
#pragma endregion

#pragma region Exported API
HRESULT MediaSourceControl::GetMFActivateSymbolicLink(
	LPWSTR* ppSymbolicLink,
	UINT32* pSymbolicLinkSize
)
{
	if (!ppSymbolicLink)
		return E_POINTER;
	if (!mpDevice)
		return E_UNEXPECTED;

	std::shared_lock<std::shared_mutex> rlock(mMutex);
	return mpDevice->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
										ppSymbolicLink,
										pSymbolicLinkSize);
}


HRESULT MediaSourceControl::GetAvaliableMediaFormat(
	DWORD streamIndex,
	DWORD typeIndex,
	MediaFormat& mediaType
)
{
	if (!mpSourceReader)
		return E_UNEXPECTED;

	CComPtr<IMFMediaType> pMFMediaType = nullptr;
	GUID subtype = GUID_NULL;
	HRESULT res = S_OK;
	UINT32 width = 0;
	UINT32 height = 0;
	UINT32 numerator = 0;
	UINT32 denominator = 0;

	std::shared_lock<std::shared_mutex> rlock(mMutex);
	res = mpSourceReader->GetNativeMediaType(streamIndex, typeIndex, &pMFMediaType);
	if (FAILED(res))
		return res;

	res = pMFMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
	if (FAILED(res))
		return res;

	res = MapMFMediaType(subtype, mediaType.Format);
	if (FAILED(res))
		return res;

	res = MFGetAttributeSize(pMFMediaType, MF_MT_FRAME_SIZE, &width, &height);
	if (FAILED(res))
		return res;

	mediaType.Width = width;
	mediaType.Height = height;


	res = MFGetAttributeSize(pMFMediaType, MF_MT_FRAME_RATE, &numerator, &denominator);

	mediaType.FrameNumerator = numerator;
	mediaType.FrameDenominator = denominator;

	return res;
}

HRESULT MediaSourceControl::GetCurrentMediaFormat(
	DWORD streamIndex,
	MediaFormat& mediaType
)
{
	HRESULT res = S_OK;
	CComPtr<IMFMediaType> pMFMediaType = nullptr;
	GUID subtype = GUID_NULL;
	UINT32 height = 0;
	UINT32 width = 0;

	if (!mpSourceReader)
		return E_UNEXPECTED;

	std::shared_lock<std::shared_mutex> rlock(mMutex);
	res = mpSourceReader->GetCurrentMediaType(streamIndex, &pMFMediaType);
	if (FAILED(res))
		return res;

	res = pMFMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
	if (FAILED(res))
		return res;

	res = MapMFMediaType(subtype, mediaType.Format);
	if (FAILED(res))
		return res;

	res = MFGetAttributeSize(pMFMediaType, MF_MT_FRAME_SIZE, &width, &height);
	mediaType.Width = width;
	mediaType.Height = height;

	return res;
}

HRESULT MediaSourceControl::GetCurrentMediaType(
	DWORD streamIndex,
	IMFMediaType** ppMediaType
)
{
	HRESULT res = S_OK;

	if (!ppMediaType)
		return E_POINTER;
	if (!mpSourceReader)
		return E_UNEXPECTED;

	std::shared_lock<std::shared_mutex> rlock(mMutex);
	return mpSourceReader->GetCurrentMediaType(streamIndex, ppMediaType);
}

HRESULT MediaSourceControl::SetCurrentMediaFormat(
	DWORD streamIndex,
	DWORD typeIndex
)
{
	HRESULT res = S_OK;
	CComPtr<IMFMediaType> pMFMediaType = nullptr;

	if (!mpSourceReader)
		return E_UNEXPECTED;

	std::unique_lock<std::shared_mutex> wlock(mMutex);
	res = mpSourceReader->GetNativeMediaType(streamIndex, typeIndex, &pMFMediaType);
	if (FAILED(res))
		return res;

	return mpSourceReader->SetCurrentMediaType(streamIndex, nullptr, pMFMediaType);
}

HRESULT MediaSourceControl::SetStreamState(
	DWORD streamIndex,
	BOOL isSelected
)
{
	HRESULT res = S_OK;

	if (!mpSourceReader)
		return E_UNEXPECTED;

	{
		std::unique_lock<std::shared_mutex> wlock(mMutex);

		if (isSelected)
			mWorkingPins.insert(streamIndex);
		else
			mWorkingPins.erase(streamIndex);

		res = mpSourceReader->SetStreamSelection(streamIndex, isSelected);
		if (FAILED(res))
			return res;
	}

	if (isSelected) {
		MediaFormat format;
		res = GetCurrentMediaFormat(streamIndex, format);

		{
			std::unique_lock<std::shared_mutex> wlock(mMutex);

			if (SUCCEEDED(res))
				mPinFormatMap[streamIndex] = format;

			return mpSourceReader->ReadSample(streamIndex, 0, NULL, NULL, NULL, NULL);
		}
	} else {
		return res;
	}
}

HRESULT MediaSourceControl::GetStreamState(
	DWORD streamIndex,
	BOOL& isSelected
)
{
	std::shared_lock<std::shared_mutex> rlock(mMutex);
	return mpSourceReader->GetStreamSelection(streamIndex, &isSelected);
}

HRESULT MediaSourceControl::SetPinDataCallback(
	void* caller,
	PinDataCallback cb
)
{
	if (!caller || !cb)
		return E_POINTER;

	bool isFound = false;

	for (auto& item : mPinDataCallbackList) {
		// If the callback function has already been registered, update it
		if (item.first == caller) {
			isFound = true;
			item.second = cb;
			break;
		}
	}

	if (!isFound) {
		mPinDataCallbackList.push_back(std::make_pair(caller, cb));
	}

	return S_OK;
}

HRESULT MediaSourceControl::SetPinSampleCallback(
	void* caller,
	PinSampleCallback cb
)
{
	if (!caller || !cb)
		return E_POINTER;

	bool isFound = false;

	for (auto& item : mPinSampleCallbackList) {
		// If the callback function has already been registered, update it
		if (item.first == caller) {
			isFound = true;
			item.second = cb;
			break;
		}
	}

	if (!isFound) {
		mPinSampleCallbackList.push_back(std::make_pair(caller, cb));
	}

	return S_OK;
}

HRESULT MediaSourceControl::ClearPinDataCallback(
	void* caller
)
{
	std::unique_lock<std::shared_mutex> wlock(mMutex);

	for (auto item = mPinDataCallbackList.begin(); item != mPinDataCallbackList.end(); ) {
		if (item->first == caller)
			item = mPinDataCallbackList.erase(item);
		else
			item += 1;
	}

	return S_OK;
}

HRESULT MediaSourceControl::ClearPinSampleCallback(
	void* caller
)
{
	std::unique_lock<std::shared_mutex> wlock(mMutex);

	for (auto item = mPinSampleCallbackList.begin(); item != mPinSampleCallbackList.end(); ) {
		if (item->first == caller)
			item = mPinSampleCallbackList.erase(item);
		else
			item += 1;
	}

	return S_OK;
}

HRESULT MediaSourceControl::GetPreviewPinIndex(DWORD& pinIndex)
{
	if (mDefaultPreviewPin != -1) {
		pinIndex = mDefaultPreviewPin;
		return S_OK;
	} else {
		return E_UNEXPECTED;
	}
}

HRESULT MediaSourceControl::GetPhotoPinIndex(DWORD& pinIndex)
{
	if (mDefaultPhotoPin != -1) {
		pinIndex = mDefaultPhotoPin;
		return S_OK;
	} else {
		return E_UNEXPECTED;
	}
}

HRESULT MediaSourceControl::GetRecordPinIndex(DWORD& pinIndex)
{
	if (mDefaultRecordPin != -1) {
		pinIndex = mDefaultRecordPin;
		return S_OK;
	} else {
		return E_UNEXPECTED;
	}
}
#pragma endregion

#pragma region Helper function
//
// Helper functions
//
HRESULT MediaSourceControl::GetMFActivateCount(
	const GUID& keyType,
	const GUID& valueType,
	UINT32* pDeviceCount
)
{
	CComPtr<IMFAttributes> pAttr = nullptr;
	IMFActivate** ppDevices = nullptr;
	HRESULT res = S_OK;

	if (!pDeviceCount)
		return E_POINTER;

	res = MFCreateAttributes(&pAttr, 1);
	if (FAILED(res))
		return res;

	res = pAttr->SetGUID(keyType, valueType);
	if (FAILED(res))
		return res;

	res = MFEnumDeviceSources(pAttr, &ppDevices, pDeviceCount);
	if (FAILED(res))
		return res;

	return S_OK;
}

HRESULT MediaSourceControl::GetMFActivate(
	const GUID& keyType,
	const GUID& valueType,
	unsigned int index,
	IMFActivate** ppDevice
)
{
	CComPtr<IMFAttributes> pAttr = nullptr;
	IMFActivate** ppDevices = nullptr;
	uint32_t deviceCount = 0;
	HRESULT res = S_OK;

	if (!ppDevice)
		return E_POINTER;

	res = MFCreateAttributes(&pAttr, 1);
	if (FAILED(res))
		return res;

	res = pAttr->SetGUID(keyType, valueType);
	if (FAILED(res))
		return res;

	res = MFEnumDeviceSources(pAttr, &ppDevices, &deviceCount);
	if (FAILED(res))
		return res;

	if (index >= deviceCount)
		return E_INVALIDARG;

	*ppDevice = ppDevices[index];
	return S_OK;
}

static std::pair<GUID, VideoFormat> MediaTypeMapper[] = {
    { std::make_pair(MFVideoFormat_RGB8,   RGB8)   },
	{ std::make_pair(MFVideoFormat_RGB24,  RGB24)  },
	{ std::make_pair(MFVideoFormat_RGB32,  RGB32)  },
	{ std::make_pair(MFVideoFormat_ARGB32, ARGB32) },
	{ std::make_pair(MFVideoFormat_NV12,   NV12)   },
	{ std::make_pair(MFVideoFormat_YUY2,   YUY2)   },
};

HRESULT MediaSourceControl::MapMFMediaType(GUID mfGuid, VideoFormat& format)
{
	bool isFound = false;

	for (size_t i = 0; i < size(MediaTypeMapper); i++) {
		if (IsEqualGUID(MediaTypeMapper[i].first, mfGuid)) {
			format = MediaTypeMapper[i].second;
			isFound = true;
		}
	}

	return isFound ? S_OK : E_INVALIDARG;
}
#pragma endregion

