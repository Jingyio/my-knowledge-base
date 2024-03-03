#ifndef _LIBCAMDEF_H_
#define _LIBCAMDEF_H_

#include <stdint.h>

#ifdef LIBCAM_EXPORTS
#define LIBCAM_API __declspec(dllexport)
#else
#define LIBCAM_API __declspec(dllimport)
#endif

typedef enum LIBCAM_API : uint64_t {
    OK = 0,
    INVALID_HANDLE,
    INVALID_PARAM,
    UNINITIALIZED,
    Error,
} ErrorCode;

typedef enum LIBCAM_API : uint64_t {
    RGB8,
    RGB24,
    RGB32,
    ARGB32,
    NV12,
    YUY2
} VideoFormat;

typedef struct LIBCAM_API {
    VideoFormat  Format;
    uint64_t     Width;
    uint64_t     Height;
    int64_t      Stride;
    uint64_t     FrameNumerator;
    uint64_t     FrameDenominator;
} MediaFormat;

typedef LIBCAM_API enum : uint64_t {
    PREVIEW = 0,
    RECORD,
    PHOTO,
} PinType;

typedef LIBCAM_API enum : uint64_t {
    STOP = 0,
    START,
    TRANSFORM,
} ActionType;

typedef void LIBCAM_API(*RenderCallback)(unsigned char*, MediaFormat);
typedef ErrorCode(*PinDataCallback)(void*, int, unsigned char*, MediaFormat);

typedef struct LIBCAM_API _CameraConfig {
    PinType         Type;
    uint64_t        FormatIndex;
    ActionType      Action;
    RenderCallback  Callback;
    const char*     FilePath;
} CameraConfig;

#define MAX_FORMAT_SIZE (128)
typedef struct LIBCAM_API {
    MediaFormat  PreviewFormatArray[MAX_FORMAT_SIZE];
    MediaFormat  RecordFormatArray[MAX_FORMAT_SIZE];
    MediaFormat  PhotoFormatArray[MAX_FORMAT_SIZE];
    uint64_t     PreviewFormatCount;
    uint64_t     RecordFormatCount;
    uint64_t     PhotoFormatCount;
    MediaFormat  CurrentPreviewFormat;
    MediaFormat  CurrentRecordFormat;
    MediaFormat  CurrentPhotoFormat;
} CameraInfo;

#endif
