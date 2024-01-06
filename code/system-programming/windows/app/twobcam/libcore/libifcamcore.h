#ifndef _LIBIFCAMCORE_H_
#define _LIBIFCAMCORE_H_

#ifdef LIBCAMCORE_EXPORTS
#define LIBCAMCORE_API __declspec(dllexport)
#else
#define LIBCAMCORE_API __declspec(dllimport)
#endif

extern "C" {

typedef enum LIBCAMCORE_API {
    OK = 0,
    INVALID_HANDLE,
    INVALID_PARAM,
    UNINITIALIZED,
    Error,
} ErrorCode;

typedef enum {
    RGB8,
    RGB24,
    RGB32,
    ARGB32,
    NV12,
    YUY2
} VideoFormat;

typedef struct LIBCAMCORE_API {
    VideoFormat Format;
    unsigned int Width;
    unsigned int Height;
    long Stride;
} MediaFormat;

typedef LIBCAMCORE_API enum {
    PREVIEW = 0,
    RECORD,
    PHOTO,
} PinType;

typedef LIBCAMCORE_API enum {
    STOP = 0,
    START,
    TRANSFORM,
} ActionType;

typedef void LIBCAMCORE_API (*RenderCallback)(unsigned char*, MediaFormat);
typedef ErrorCode (*PinDataCallback)(void*, int, unsigned char*, MediaFormat);

typedef struct LIBCAMCORE_API {
    PinType        Type;
    ActionType     Action;
    RenderCallback Callback;
    const char*    FilePath;
} CameraConfig;

#define MAX_FORMAT_SIZE (128)
typedef struct LIBCAMCORE_API {
    MediaFormat  PreviewFormatArray[MAX_FORMAT_SIZE];
    MediaFormat  RecordFormatArray[MAX_FORMAT_SIZE];
    MediaFormat  PhotoFormatArray[MAX_FORMAT_SIZE];
    unsigned int PreviewFormatCount;
    unsigned int RecordFormatCount;
    unsigned int PhotoFormatCount;
} CameraInfo;


typedef struct LIBCAMCORE_API {
    void* Unused;
    void* UnusedData;
} CameraHandle;

typedef struct LIBCAMCORE_API {
    void* Unused;
} PlatformHandle;

ErrorCode LIBCAMCORE_API GetCameraCount (unsigned int*);
ErrorCode LIBCAMCORE_API OpenCamera     (unsigned int, CameraHandle*);
ErrorCode LIBCAMCORE_API CloseCamera    (CameraHandle);
ErrorCode LIBCAMCORE_API ConfigureCamera(CameraHandle, CameraConfig);
ErrorCode LIBCAMCORE_API QueryCameraInfo(CameraHandle, CameraInfo*);
}

#endif