#include "libcamhalif.h"
#include "libcamcoreif.h"
#include "liblog.h"

static ErrorCode ConfigHandler(CameraHandle handle, CameraConfig config)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    if (config.Type == PREVIEW) {
        if (config.FormatIndex != -1)
            ((NSCAM::ICamCore*)handle.Unused)->SetCurrentFormat(PinType::PREVIEW, config.FormatIndex);

        if (config.Action == START)
            return ((NSCAM::ICamCore*)handle.Unused)->StartPreview(config.Callback);
        else if (config.Action == STOP)
            return ((NSCAM::ICamCore*)handle.Unused)->StopPreview();
    }

    if (config.Type == PHOTO) {
        if (config.FormatIndex != -1)
            ErrorCode res = ((NSCAM::ICamCore*)handle.Unused)->SetCurrentFormat(PinType::PHOTO, config.FormatIndex);

        return ((NSCAM::ICamCore*)handle.Unused)->TakePhoto(config.FilePath);
    }

    if (config.Type == RECORD) {
        if (config.FormatIndex != -1)
            ErrorCode res = ((NSCAM::ICamCore*)handle.Unused)->SetCurrentFormat(PinType::PHOTO, config.FormatIndex);

        if (config.Action == START)
            return ((NSCAM::ICamCore*)handle.Unused)->StartRecord(config.FilePath);
        else if (config.Action == STOP)
            return ((NSCAM::ICamCore*)handle.Unused)->StopRecord();
    }

    return OK;
}

static ErrorCode InfoHandler(CameraHandle handle, CameraInfo& info)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    NSCAM::ICamCore* pCamCore = ((NSCAM::ICamCore*)handle.Unused);

    memset(info.PreviewFormatArray, 0, MAX_FORMAT_SIZE * sizeof(MediaFormat));
    memset(info.RecordFormatArray, 0, MAX_FORMAT_SIZE * sizeof(MediaFormat));
    memset(info.PhotoFormatArray, 0, MAX_FORMAT_SIZE * sizeof(MediaFormat));
    info.PreviewFormatCount = 0;
    info.RecordFormatCount = 0;
    info.PhotoFormatCount = 0;

    if (pCamCore->GetCurrentFormat(PinType::PREVIEW, &info.CurrentPreviewFormat))
        memset(&info.CurrentPreviewFormat, 0, sizeof(MediaFormat));
    if (pCamCore->GetCurrentFormat(PinType::RECORD, &info.CurrentRecordFormat))
        memset(&info.CurrentRecordFormat, 0, sizeof(MediaFormat));
    if (pCamCore->GetCurrentFormat(PinType::PHOTO, &info.CurrentPhotoFormat))
        memset(&info.CurrentPhotoFormat, 0, sizeof(MediaFormat));

    // Get supported media formats for each pin
    ErrorCode previewRes = pCamCore->GetSupportedFormat(PREVIEW, info.PreviewFormatArray, &info.PreviewFormatCount);
    ErrorCode recordRes = pCamCore->GetSupportedFormat(RECORD, info.RecordFormatArray, &info.RecordFormatCount);
    ErrorCode photoRes = pCamCore->GetSupportedFormat(PHOTO, info.PhotoFormatArray, &info.PhotoFormatCount);

    if (!previewRes && !recordRes && !photoRes)
        return OK;
    else
        return INVALID_PARAM;
}


ErrorCode GetCameraCount(unsigned int* pCount)
{
    return NSCAM::ICamCore::GetCameraCount(pCount);
}

ErrorCode OpenCamera(unsigned int cameraIndex, CameraHandle* pHandle)
{
    if (!pHandle)
        return INVALID_HANDLE;

    unsigned int count = 0;
    ErrorCode res = OK;

    do {
        res = GetCameraCount(&count);
        if (res != OK)
            break;

        if (cameraIndex >= count) {
            res = INVALID_PARAM;
            break;
        }

        pHandle->Unused = NSCAM::ICamCore::CreateInstance(cameraIndex);
    } while (0);

    TRACE_AND_RETURN(res);
}

ErrorCode CloseCamera(CameraHandle handle)
{
    if (!handle.Unused)
        return INVALID_HANDLE;

    delete handle.Unused;
    handle.Unused = nullptr;

    return OK;
}

ErrorCode ConfigureCamera(CameraHandle handle, CameraConfig config)
{
    return ConfigHandler(handle, config);
}

ErrorCode QueryCameraInfo(CameraHandle handle, CameraInfo* pInfo)
{
    return InfoHandler(handle, *pInfo);
}
