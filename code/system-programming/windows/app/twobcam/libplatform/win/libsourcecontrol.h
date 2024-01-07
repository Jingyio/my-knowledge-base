#ifndef _LIBSOURCECONTROLWIN_H_
#define _LIBSOURCECONTROLWIN_H_

#include <Windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <atlbase.h>
#include <memory>
#include <mfreadwrite.h>
#include <mutex>
#include <vector>
#include <map>
#include <shared_mutex>
#include "../../libcore/libcamdef.h"

class MediaSourceControl : public IMFSourceReaderCallback {
public:
    enum class PinType {
        PREVIEW,
        IMAGE,
        RECORD,
        PINTYPE_END,
    };

public:
    static MediaSourceControl* CreateInstance           (unsigned int);
    explicit                   MediaSourceControl       (IMFActivate*);
                               ~MediaSourceControl      (void) = default;

    /* IUnknown interfaces */
    HRESULT                    QueryInterface           (REFIID, void**);
    ULONG                      AddRef                   (void);
    ULONG                      Release                  (void);

    /* IMFSourceReaderCallback interfaces */
    HRESULT                    OnReadSample             (HRESULT, DWORD, DWORD, LONGLONG, IMFSample*);
    HRESULT                    OnFlush                  (DWORD);
    HRESULT                    OnEvent                  (DWORD, IMFMediaEvent*);

    /* Exported API */
    /* TODO: Make them platform independent */
    HRESULT                    GetMFActivateSymbolicLink(LPWSTR*, UINT32*);
    HRESULT                    GetAvaliableMediaFormat  (DWORD, DWORD, MediaFormat&);
    HRESULT                    SetCurrentMediaFormat    (DWORD, DWORD);
    HRESULT                    GetCurrentMediaFormat    (DWORD, MediaFormat&);
    HRESULT                    SetStreamState           (DWORD, bool);
    HRESULT                    SetPinDataCallback       (void*, PinDataCallback cb);
    HRESULT                    ClearPinDataCallback     (void*);
    HRESULT                    GetPreviewPinIndex       (DWORD&);
    HRESULT                    GetPhotoPinIndex         (DWORD&);
    HRESULT                    GetRecordPinIndex        (DWORD&);

    /* Helper functions */
    static HRESULT             GetMFActivateCount       (const GUID&, const GUID&, UINT32*);
    static HRESULT             GetMFActivate            (const GUID&, const GUID&, unsigned int, IMFActivate**);
    static HRESULT             MapMFMediaType           (GUID, VideoFormat&);

    static std::mutex                                   sInstLock;
    static IMFActivate**                                sMFActivates;

protected:
    ULONG                                               mRefCount;
    CComPtr<IMFActivate>                                mpDevice;
    CComPtr<IMFMediaSource>                             mpMediaSource;
    CComPtr<IMFSourceReader>                            mpSourceReader;
    mutable std::shared_mutex                           mMutex;
    std::map<unsigned int, PinType>                     mPinTypeMap;
    std::map<unsigned int, MediaFormat>                 mPinFormatMap;
    std::vector<std::pair<void*, PinDataCallback>>      mPinDataCallbackList;
    int                                                 mDefaultPreviewPin;
    int                                                 mDefaultRecordPin;
    int                                                 mDefaultPhotoPin;
};

#endif
