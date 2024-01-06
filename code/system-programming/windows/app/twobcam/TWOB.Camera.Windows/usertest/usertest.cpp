#include <iostream>
#include <format>
#include <cstdarg>
#include "../../libCore/liblog.h"
#include "../../libCore/libifcamcore.h"

CameraHandle handle = { 0 };

void Render(unsigned char* pBuf, MediaFormat type)
{
    int col = 0, row = 0;
    static int sCount = 0;
    ErrorCode ret = OK;

    sCount++;

    if (sCount >= 30) {
        sCount = 0;

        printf("format: %d, %d X %d\n", type.Format, type.Width, type.Height);

        for (int row = 0; row < 10; row++) {
            for (int col = 0; col < 10; col++) {
                printf("%d ", pBuf[row * type.Width + col]);
            }
            printf("\n");
        }
        printf("\n");

        CameraConfig config = {
            .Type = PREVIEW,
            .Action = STOP,
            .Callback = Render,
        };

        ret = ConfigureCamera(handle, config);
    }
}

int main()
{
    unsigned int count = 0;
    ErrorCode ret = OK;

    ret = GetCameraCount(&count);
    if (count > 0) {
        ret = OpenCamera(0, &handle);
        if (ret == OK) {

            // Dump supported formats
            CameraInfo info;
            QueryCameraInfo(handle, &info);
            for (int i = 0; i < info.PreviewFormatCount; i++) {
                printf("PREVIEW %d: type %d, %d X %d\n", i, info.PreviewFormatArray[i].Format, info.PreviewFormatArray[i].Width, info.PreviewFormatArray[i].Height);
            }
            for (int i = 0; i < info.RecordFormatCount; i++) {
                printf("RECORD %d: type %d, %d X %d\n", i, info.RecordFormatArray[i].Format, info.RecordFormatArray[i].Width, info.RecordFormatArray[i].Height);
            }
            for (int i = 0; i < info.PhotoFormatCount; i++) {
                printf("PHOTO %d: type %d, %d X %d\n", i, info.PhotoFormatArray[i].Format, info.PhotoFormatArray[i].Width, info.PhotoFormatArray[i].Height);
            }

            CameraConfig config = {
                .Type  = PREVIEW,
                .Action = START,
                .Callback = Render,
            };

            ret = ConfigureCamera(handle, config);

            CameraConfig config2 = {
                .Type = PHOTO,
                .FilePath = "Suliko.bmp",
            };
            ret = ConfigureCamera(handle, config2);
        }
    }

    while (1)
    {

    }
}
