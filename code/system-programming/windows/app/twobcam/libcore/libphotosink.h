#ifndef _LIBPHOTOSINK_H_
#define _LIBPHOTOSINK_H_

#include <stdint.h>
#include <string>
#include <fstream>
#include "libifcamcore.h"

class Bmp {
public:
    Bmp(std::string);
    ~Bmp(void);

    bool CreateFile(std::string name);
    void Write(uint32_t offset, const char* buffer, uint32_t length);
    void Read(uint32_t offset, char* buffer, uint32_t length);
    void Save(uint32_t width, uint32_t height);
    void Close(void);
    void operator << (uint8_t data);
    void operator << (uint16_t data);
    void operator << (uint32_t data);
    void operator << (const char* ptr_data);

    struct FileHeader {
        uint16_t Type;
        uint32_t FileSize;
        uint32_t Reserved;
        uint32_t DataOffset;
    };

    struct InfoHeader {
        uint32_t InfoSize;
        uint32_t Width;
        uint32_t Height;
        uint16_t Planes;
        uint16_t BitCount;
        uint32_t Compression;
        uint32_t DataSize;
        uint32_t XpixelPerMeter;
        uint32_t YpixelPerMeter;
        uint32_t Color;
        uint32_t ColorImportant;
    };

private:
    std::string  mFileName;
    std::fstream mFileStream;
    uint32_t     mDataCount;
};

class PhotoSink {
public:
    explicit PhotoSink(std::string);
    ~PhotoSink(void) = default;

    void LoadDataFromRGBA(unsigned char*, unsigned int, unsigned int);
    void LoadDataFromYUY2(unsigned char*, unsigned int, unsigned int);
    void LoadDataFromNV12(unsigned char*, unsigned int, unsigned int);
    void SaveFile(unsigned int, unsigned int);

private:
    Bmp mBmp;
};

#endif