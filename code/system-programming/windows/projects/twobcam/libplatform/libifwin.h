#ifndef _LIBINTERFACEWIN_H_
#define _LIBINTERFACEWIN_H_

#ifdef LIBCAMPLATFORM_EXPORTS
#define LIBCAMPLATFORM_API __declspec(dllexport)
#else
#define LIBCAMPLATFORM_API __declspec(dllimport)
#endif

#include "../libcore/libifcamcore.h"

extern "C" {

ErrorCode LIBCAMPLATFORM_API WinGetCameraCount               (unsigned int*);
ErrorCode LIBCAMPLATFORM_API WinGetCameraSymbolicLink        (PlatformHandle, wchar_t**, size_t*);
ErrorCode LIBCAMPLATFORM_API WinOpenCamera                   (unsigned int, PlatformHandle*);
ErrorCode LIBCAMPLATFORM_API WinCloseCamera                  (PlatformHandle);
ErrorCode LIBCAMPLATFORM_API WinSetCameraState               (PlatformHandle, unsigned int, bool);

ErrorCode LIBCAMPLATFORM_API WinRegisterCameraDataCallback   (PlatformHandle, void*, PinDataCallback);
ErrorCode LIBCAMPLATFORM_API WinClearCameraDataCallback      (PlatformHandle, void*);

ErrorCode LIBCAMPLATFORM_API WinGetCameraSupportedMediaFormat(PlatformHandle, unsigned int, unsigned int, MediaFormat&);
ErrorCode LIBCAMPLATFORM_API WinSetCameraMediaFormat         (PlatformHandle, unsigned int, unsigned int);
ErrorCode LIBCAMPLATFORM_API WinGetCameraMediaFormat         (PlatformHandle, unsigned int, MediaFormat&);
ErrorCode LIBCAMPLATFORM_API WinGetCameraPreviewPinIndex     (PlatformHandle, unsigned int&);
ErrorCode LIBCAMPLATFORM_API WinGetCameraPhotoPinIndex       (PlatformHandle, unsigned int&);
ErrorCode LIBCAMPLATFORM_API WinGetCameraRecordPinIndex      (PlatformHandle, unsigned int&);
}

#endif