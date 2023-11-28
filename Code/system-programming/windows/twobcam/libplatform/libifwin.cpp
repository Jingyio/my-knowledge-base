#include "libifwin.h"
#include "libsourcecontrolwin.h"

static IMFActivate** ppMFActivates = nullptr;

static void WinGetAllActivates(
    void
)
{
    HRESULT res = S_OK;
    delete[] ppMFActivates;

    ppMFActivates = new IMFActivate * [128];
    memset(ppMFActivates, 0, 128 * sizeof(IMFActivate*));

    for (int i = 0; ; i++) {
        res = MediaSourceControl::GetMFActivate(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                                                MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                i,
                                                &ppMFActivates[i]);
        if (FAILED(res))
            break;
    }
}

ErrorCode WinGetCameraCount(
    unsigned int* pCount
)
{
    if (!pCount)
        return INVALID_PARAM;

    HRESULT res = S_OK;
    UINT32 count = 0;

    res = MediaSourceControl::GetMFActivateCount(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                                                 MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                 &count);

    if (SUCCEEDED(res)) {
        *pCount = count;
    }

    return SUCCEEDED(res) ? OK : Error;
}

ErrorCode WinGetCameraSymbolicLink(
    PlatformHandle handle,
    wchar_t** pSymbolic,
    size_t* size
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    HRESULT res = ((MediaSourceControl*)(handle.Unused))->GetMFActivateSymbolicLink(pSymbolic, (UINT32*)size);

    return SUCCEEDED(res) ? OK : Error;
}

ErrorCode WinOpenCamera(
    unsigned int cameraIndex,
    PlatformHandle* pHandle
)
{
    if (!pHandle)
        return INVALID_HANDLE;

    unsigned int count = 0;
    HRESULT res = S_OK;

    do {
        if (WinGetCameraCount(&count) != OK) {
            res = E_UNEXPECTED;
            break;
        }

        if (cameraIndex >= count) {
            res = E_INVALIDARG;
            break;
        }

        if (!ppMFActivates)
            WinGetAllActivates();

        if (ppMFActivates) {
            pHandle->Unused = MediaSourceControl::CreateInstance(ppMFActivates[cameraIndex]);
        }
    } while (0);

    return SUCCEEDED(res) ? OK : Error;
}

ErrorCode WinCloseCamera(
    PlatformHandle handle
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    delete (MediaSourceControl*)handle.Unused;
    handle.Unused = nullptr;

    return OK;
}

ErrorCode WinSetCameraState(
    PlatformHandle handle,
    unsigned int pinIndex,
    bool isSelected
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    return SUCCEEDED(((MediaSourceControl*)handle.Unused)->SetStreamState(pinIndex, isSelected)) ? OK : Error;
}

ErrorCode WinRegisterCameraDataCallback(
    PlatformHandle handle,
    void *caller,
    PinDataCallback cb
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    if (!cb)
        return INVALID_PARAM;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;
    if (!p)
        return Error;

    return SUCCEEDED(p->SetPinDataCallback(caller, cb)) ? OK : Error;
}

ErrorCode WinClearCameraDataCallback(
    PlatformHandle handle,
    void* caller
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;

    return SUCCEEDED(p->ClearPinDataCallback(caller)) ? OK : Error;
}

ErrorCode WinGetCameraSupportedMediaFormat(
    PlatformHandle handle,
    unsigned int pinIndex,
    unsigned int typeIndex,
    MediaFormat& mediaFormat
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;

    return SUCCEEDED(p->GetAvaliableMediaFormat(pinIndex, typeIndex, mediaFormat)) ? OK : Error;
}

ErrorCode WinSetCameraMediaFormat(
    PlatformHandle handle,
    unsigned int pinIndex,
    unsigned int typeIndex
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;

    return SUCCEEDED(p->SetCurrentMediaFormat(pinIndex, typeIndex)) ? OK : Error;
}

ErrorCode WinGetCameraMediaFormat(
    PlatformHandle handle,
    unsigned int pinIndex,
    MediaFormat& mediaFormat
)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;

    return SUCCEEDED(p->GetCurrentMediaFormat(pinIndex, mediaFormat)) ? OK : Error;
}

ErrorCode WinGetCameraPreviewPinIndex(
    PlatformHandle handle,
    unsigned int& pinIndex
)
{
    DWORD pin = 0;
    HRESULT res = S_OK;

    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;
    res = p->GetPreviewPinIndex(pin);
    if (SUCCEEDED(res)) {
        pinIndex = pin;
        return OK;
    } else {
        return Error;
    }
}

ErrorCode WinGetCameraPhotoPinIndex(
    PlatformHandle handle,
    unsigned int& pinIndex
)
{
    DWORD pin = 0;
    HRESULT res = S_OK;

    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;
    res = p->GetPhotoPinIndex(pin);
    if (SUCCEEDED(res)) {
        pinIndex = pin;
        return OK;
    } else {
        return Error;
    }
}

ErrorCode WinGetCameraRecordPinIndex(
    PlatformHandle handle,
    unsigned int& pinIndex
)
{
    DWORD pin = 0;
    HRESULT res = S_OK;

    if (!handle.Unused)
        return INVALID_HANDLE;

    MediaSourceControl* p = (MediaSourceControl*)handle.Unused;
    res = p->GetRecordPinIndex(pin);
    if (SUCCEEDED(res)) {
        pinIndex = pin;
        return OK;
    } else {
        return Error;
    }
}