#ifndef _LIBCAMDEF_H_
#define _LIBCAMDEF_H_

#ifdef LIBCAM_EXPORTS
#define LIBCAM_API __declspec(dllexport)
#else
#define LIBCAM_API __declspec(dllimport)
#endif

typedef enum LIBCAM_API {
    OK = 0,
    INVALID_HANDLE,
    INVALID_PARAM,
    UNINITIALIZED,
    Error,
} ErrorCode;

typedef enum LIBCAM_API {
    RGB8,
    RGB24,
    RGB32,
    ARGB32,
    NV12,
    YUY2
} VideoFormat;

typedef struct LIBCAM_API {
    VideoFormat Format;
    unsigned int Width;
    unsigned int Height;
    long Stride;
} MediaFormat;

typedef LIBCAM_API enum {
    PREVIEW = 0,
    RECORD,
    PHOTO,
} PinType;

typedef LIBCAM_API enum {
    STOP = 0,
    START,
    TRANSFORM,
} ActionType;

typedef void LIBCAM_API(*RenderCallback)(unsigned char*, MediaFormat);
typedef ErrorCode(*PinDataCallback)(void*, int, unsigned char*, MediaFormat);

typedef struct LIBCAM_API {
    PinType        Type;
    ActionType     Action;
    RenderCallback Callback;
    const char* FilePath;
} CameraConfig;

#define MAX_FORMAT_SIZE (128)
typedef struct LIBCAM_API {
    MediaFormat  PreviewFormatArray[MAX_FORMAT_SIZE];
    MediaFormat  RecordFormatArray[MAX_FORMAT_SIZE];
    MediaFormat  PhotoFormatArray[MAX_FORMAT_SIZE];
    unsigned int PreviewFormatCount;
    unsigned int RecordFormatCount;
    unsigned int PhotoFormatCount;
} CameraInfo;

#endif
