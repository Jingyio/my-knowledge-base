#include <iostream>
#include <format>
#include <cstdarg>
#include "../../libutil/liblog.h"
#include "../../libCore/libcamhalif.h"
#include "../../libCore/libcamdef.h"
#include <Windows.h>

CameraHandle handle = { 0 };

void Render(unsigned char* pBuf, MediaFormat type)
{
    int col = 0, row = 0;
    static int sCount = 0;
    ErrorCode ret = OK;

    sCount++;

    if (sCount >= 30) {
        sCount = 0;

        printf("format: %lld, %lld X %lld\n", type.Format, type.Width, type.Height);

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

            //for (int i = 0; i < info.PreviewFormatCount; i++) {
            //    printf("PREVIEW %d: type %d, %d X %d, frame %d\n",
            //        i,
            //        info.PreviewFormatArray[i].Format,
            //        info.PreviewFormatArray[i].Width,
            //        info.PreviewFormatArray[i].Height,
            //        info.PreviewFormatArray[i].FrameNumerator / info.PreviewFormatArray[i].FrameDenominator);
            //}

            for (int i = 0; i < info.PhotoFormatCount; i++) {
                printf("PHOTO %d: type %lld, %lld X %lld, frame %lld\n",
                    i,
                    info.PhotoFormatArray[i].Format,
                    info.PhotoFormatArray[i].Width,
                    info.PhotoFormatArray[i].Height,
                    info.PhotoFormatArray[i].FrameNumerator / info.PhotoFormatArray[i].FrameDenominator);
            }

            //printf("Current preview format: type %d, %d X %d, %d fps\n",
            //    info.CurrentPreviewFormat.Format,
            //    info.CurrentPreviewFormat.Width,
            //    info.CurrentPreviewFormat.Height,
            //    info.CurrentPreviewFormat.FrameNumerator / info.CurrentPreviewFormat.FrameDenominator);

            //printf("Current record format: type %d, %d X %d, %d fps\n",
            //    info.CurrentRecordFormat.Format,
            //    info.CurrentRecordFormat.Width,
            //    info.CurrentRecordFormat.Height,
            //    info.CurrentRecordFormat.FrameNumerator / info.CurrentRecordFormat.FrameDenominator);

            CameraConfig config = {
                .Type  = PREVIEW,
                .Action = START,
                .FormatIndex = -1,
                .Callback = Render,
            };
            ret = ConfigureCamera(handle, config);

            CameraConfig config2 = {
                .Type = PHOTO,
                //.FormatIndex = 2,
                .FormatIndex = -1,
                .FilePath = "Suliko.bmp",
            };
            ret = ConfigureCamera(handle, config2);

            CameraConfig config3 = {
                .Type = RECORD,
                .Action = START,
                .FilePath = "new.wmv",
            };
            ret = ConfigureCamera(handle, config3);

            Sleep(25000);

            CameraConfig config4 = {
                .Type = RECORD,
                .Action = STOP,
            };
            ret = ConfigureCamera(handle, config4);
        }
    }

    while (1)
    {

    }
}
