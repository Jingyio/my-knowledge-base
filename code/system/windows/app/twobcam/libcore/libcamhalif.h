#ifndef _LIBCAMHALIF_H_
#define _LIBCAMHALIF_H_

#include "libcamdef.h"

extern "C" {

typedef struct LIBCAM_API {
    void* Unused;
} CameraHandle;

ErrorCode LIBCAM_API GetCameraCount (unsigned int*);
ErrorCode LIBCAM_API OpenCamera     (unsigned int, CameraHandle*);
ErrorCode LIBCAM_API CloseCamera    (CameraHandle);
ErrorCode LIBCAM_API ConfigureCamera(CameraHandle, CameraConfig);
ErrorCode LIBCAM_API QueryCameraInfo(CameraHandle, CameraInfo*);

}       // extern "C"
#endif  // #ifndef _LIBCAMHALIF_H_