#ifndef _LIBBMP_H_
#define _LIBBMP_H_

#include <stdint.h>
#include <string>
#include <fstream>
#include "libcamdef.h"

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

#endif