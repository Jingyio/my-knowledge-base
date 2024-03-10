#include "libformatconverter.h"

inline unsigned char Clip(int data)
{
    if (data < 0)
        return 0;
    else if (data > 255)
        return 255;
    else
        return data;
}

ErrorCode YUY2toRGBA(unsigned char* pSrc, unsigned long long width, unsigned long long height, long long stride, unsigned char* pDest)
{
    if (!pSrc || !pDest)
        return INVALID_PARAM;

    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int j = 0; j < width / 2; j++) {
            int y0 = pSrc[0];
            int u0 = pSrc[1];
            int y1 = pSrc[2];
            int v0 = pSrc[3];
            pSrc += 4;

            int c = y0 - 16;
            int d = u0 - 128;
            int e = v0 - 128;

            pDest[0] = Clip((298 * c + 516 * d + 128) >> 8);
            pDest[1] = Clip((298 * c - 100 * d - 208 * e + 128) >> 8);
            pDest[2] = Clip((298 * c + 409 * e + 128) >> 8);
            pDest[3] = 255;
            c = y1 - 16;
            pDest[4] = Clip((298 * c + 516 * d + 128) >> 8);
            pDest[5] = Clip((298 * c - 100 * d - 208 * e + 128) >> 8);
            pDest[6] = Clip((298 * c + 409 * e + 128) >> 8);
            pDest[7] = 255;
            pDest += 8;
        }
    }

    return OK;
}

ErrorCode NV12toRGBA(unsigned char* pSrc, unsigned long long width, unsigned long long height, long long stride, unsigned char* pDest)
{
    if (!pSrc || !pDest)
        return INVALID_PARAM;

    unsigned char* pUV = pSrc + stride * height;
    int c00 = 0;
    int c01 = 0;
    int c10 = 0;
    int c11 = 0;
    int c1 = 0;
    int d = 0;
    int e = 0;

    for (unsigned int row = 0; row < height; row += 2) {
        unsigned char* pSrcLine0 = pSrc + row * stride;
        unsigned char* pSrcLine1 = pSrcLine0 + stride;
        unsigned char* pDestLine = pDest + row * width * 4;
        unsigned char alpha = 255;

        for (unsigned int col = 0; col < width; col += 2) {
            c00 = *(pSrcLine0 + 0) - 16;
            c01 = *(pSrcLine0 + 1) - 16;
            c10 = *(pSrcLine1 + 0) - 16;
            c11 = *(pSrcLine1 + 1) - 16;
            e = *(pUV + 0) - 128;
            d = *(pUV + 1) - 128;

            *(pDestLine + 0) = Clip((298 * c00 + 409 * e + 128) >> 8);
            *(pDestLine + 1) = Clip((298 * c00 - 100 * d - 208 * e + 128) >> 8);
            *(pDestLine + 2) = Clip((298 * c00 + 516 * d + 128) >> 8);
            *(pDestLine + 3) = alpha;

            *(pDestLine + 4) = Clip((298 * c01 + 409 * e + 128) >> 8);
            *(pDestLine + 5) = Clip((298 * c01 - 100 * d - 208 * e + 128) >> 8);
            *(pDestLine + 6) = Clip((298 * c01 + 516 * d + 128) >> 8);
            *(pDestLine + 7) = alpha;

            *(pDestLine + width * 4 + 0) = Clip((298 * c10 + 409 * e + 128) >> 8);
            *(pDestLine + width * 4 + 1) = Clip((298 * c10 - 100 * d - 208 * e + 128) >> 8);
            *(pDestLine + width * 4 + 2) = Clip((298 * c10 + 516 * d + 128) >> 8);
            *(pDestLine + width * 4 + 3) = alpha;

            *(pDestLine + width * 4 + 4) = Clip((298 * c11 + 409 * e + 128) >> 8);
            *(pDestLine + width * 4 + 5) = Clip((298 * c11 - 100 * d - 208 * e + 128) >> 8);
            *(pDestLine + width * 4 + 6) = Clip((298 * c11 + 516 * d + 128) >> 8);
            *(pDestLine + width * 4 + 7) = alpha;

            pDestLine += 8;
            pUV += 2;
            pSrcLine0 += 2;
            pSrcLine1 += 2;
        }
        pUV += (stride - width);
    }

    return OK;
}