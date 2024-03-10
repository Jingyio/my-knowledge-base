#ifndef _LIBMFSOURCE_H_
#define _LIBMFSOURCE_H_

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
#include <set>
#include "libcamdef.h"

typedef ErrorCode(*PinSampleCallback)(void*, DWORD, DWORD, IMFSample*, LONGLONG);

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
    HRESULT                    GetMFActivateSymbolicLink(LPWSTR*, UINT32*);
    HRESULT                    GetAvaliableMediaFormat  (DWORD, DWORD, MediaFormat&);
    HRESULT                    SetCurrentMediaFormat    (DWORD, DWORD);
    HRESULT                    GetCurrentMediaFormat    (DWORD, MediaFormat&);
    HRESULT                    GetCurrentMediaType      (DWORD, IMFMediaType**);
    HRESULT                    SetStreamState           (DWORD, BOOL);
    HRESULT                    GetStreamState           (DWORD, BOOL&);
    HRESULT                    SetPinSampleCallback     (void*, PinSampleCallback);
    HRESULT                    ClearPinSampleCallback   (void*);
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
    std::vector<std::pair<void*, PinSampleCallback>>    mPinSampleCallbackList;
    std::set<DWORD>                                     mWorkingPins;
    int                                                 mDefaultPreviewPin;
    int                                                 mDefaultRecordPin;
    int                                                 mDefaultPhotoPin;
};

#endif
