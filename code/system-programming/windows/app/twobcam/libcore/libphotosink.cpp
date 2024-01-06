#include "libphotosink.h"
#include "libformatconverter.h"
#include <iostream>

Bmp::Bmp(std::string name)
    : mFileName(name)
    , mFileStream()
    , mDataCount(0)
{

}

Bmp::~Bmp(void)
{
    if (mFileStream.is_open())
        mFileStream.close();
}

/**
 * @brief   Create a new file and point the file pointer to the data region.
 *
 * @param   name    bmp file name
 * @retval  true    success
 * @retval  false   fail
 */
bool Bmp::CreateFile(std::string name)
{
    mFileName = name;

    if (!mFileStream.is_open()) {
        mFileStream.open(mFileName, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);
        if (!mFileStream.is_open()) {
            std::cout << "open failed" << std::endl;
            return false;
        }

        // Point to the beginning of the data region
        mFileStream.seekp(54, std::ios::beg);
    }

    return true;
}

/**
 * @brief   Write a byte to the bmp file data region. This operation will increase mDataCount.
 *          It is necessary to ensure that the value of mDataCount is not changed when using
 *          this operator to write non data region content.
 *
 * @param   data    a byte
 */
void Bmp::operator << (uint8_t data)
{
    if (!mFileStream.is_open()) {
        if (!CreateFile(mFileName))
            return;
    }

    mFileStream << data;
    mDataCount += 1;
}

void Bmp::operator << (uint16_t data)
{
    *this << (uint8_t)(data & 0xff);
    *this << (uint8_t)((data >> 8) & 0xff);
}

void Bmp::operator << (uint32_t data)
{
    *this << (uint16_t)(data & 0xffff);
    *this << (uint16_t)((data >> 16) & 0xffff);
}

void Bmp::operator << (const char* ptr_data)
{
    if (!ptr_data)
        return;

    if (!mFileStream.is_open()) {
        if (!CreateFile(mFileName))
            return;
    }

    while (*ptr_data != '\0') {
        *this << (uint8_t)*ptr_data;
        ptr_data += 1;
    }
}

/**
 * @brief   Write data to the file.
 *
 * @param   offset  from the beginning of the file
 * @param   buffer  where to write
 * @param   length  length to be written
 */
void Bmp::Write(uint32_t offset, const char* buffer, uint32_t length)
{
    if (!buffer)
        return;

    if (!mFileStream.is_open()) {
        if (!CreateFile(mFileName))
            return;
    }

    auto originPos = mFileStream.tellp();
    mFileStream.seekp(offset, std::ios::beg);
    mFileStream.write(buffer, length);
    mFileStream.seekp(originPos);
}

/**
 * @brief   Read data from the file.
 *
 * @param   offset  from the beginning of the file
 * @param   buffer  where to read
 * @param   length  length to be read
 */
void Bmp::Read(uint32_t offset, char* buffer, uint32_t length)
{
    if (!buffer)
        return;

    if (!mFileStream.is_open()) {
        if (!CreateFile(mFileName))
            return;
    }

    auto originPos = mFileStream.tellg();
    mFileStream.seekg(offset, std::ios::beg);
    mFileStream.read(buffer, length);
    mFileStream.seekg(originPos);
}

/**
 * @brief   Construct the bmp header. May pad the data region with zero.
 *
 * @param   width
 * @param   height
 */
void Bmp::Save(uint32_t width, uint32_t height)
{
    if (!mFileStream.is_open())
        return;

    FileHeader fileHeader = { 0 };
    InfoHeader infoHeader = { 0 };
    auto originPos = mFileStream.tellp();

    // Write File Header
    mFileStream.seekp(0, std::ios::beg);
    fileHeader.Type = 19778;
    fileHeader.FileSize = (14 + sizeof(InfoHeader) + width * height * 3);
    fileHeader.Reserved = 0;
    fileHeader.DataOffset = 14 + sizeof(InfoHeader);

    *this << (uint16_t)fileHeader.Type;
    *this << (uint32_t)fileHeader.FileSize;
    *this << (uint32_t)fileHeader.Reserved;
    *this << (uint32_t)fileHeader.DataOffset;
    mDataCount -= 14;

    // Write Info Header
    infoHeader.InfoSize = sizeof(InfoHeader);
    infoHeader.Width = width;
    infoHeader.Height = height;
    infoHeader.Planes = 1;
    infoHeader.BitCount = 24;
    infoHeader.Compression = 0;
    infoHeader.DataSize = width * height * 3;
    infoHeader.XpixelPerMeter = 0;
    infoHeader.YpixelPerMeter = 0;
    infoHeader.Color = 0;
    infoHeader.ColorImportant = 0;
    Write(14, (const char*)&infoHeader, sizeof(InfoHeader));

    // Write Padding
    mFileStream.seekp(0, std::ios::end);
    while (mDataCount < infoHeader.DataSize) {
        *this << (uint8_t)0;
    }

    mFileStream.seekp(originPos);
}

void Bmp::Close(void)
{
    if (mFileStream.is_open())
        mFileStream.close();
}

int main(int argc, char* argv[])
{
    Bmp bmp("unname.bmp");
    uint32_t width = 1024;
    uint32_t height = 1024;
    uint8_t r = 199;
    uint8_t g = 237;
    uint8_t b = 204;

    bmp.CreateFile("test.bmp");

    for (uint32_t i = 0; i < width; i++) {
        for (uint32_t j = 0; j < height; j++) {
            bmp << r;
            bmp << g;
            bmp << b;
        }
    }

    bmp.Save(width, height);
    bmp.Close();

    return 0;
}

PhotoSink::PhotoSink(std::string fileName)
    : mBmp("")
{
    mBmp.CreateFile(fileName);
}

void PhotoSink::LoadDataFromRGBA(unsigned char* pBuf, unsigned int width, unsigned int height)
{
    for (unsigned int i = 0; i < width; i++) {
        for (unsigned int j = 0; j < height; j++) {
            mBmp << pBuf[0];
            mBmp << pBuf[1];
            mBmp << pBuf[2];

            pBuf += 4;
        }
    }
}

void PhotoSink::LoadDataFromYUY2(unsigned char* pBuf, unsigned int width, unsigned int height)
{

}

void PhotoSink::LoadDataFromNV12(unsigned char* pBuf, unsigned int width, unsigned int height)
{

}

void PhotoSink::SaveFile(unsigned int width, unsigned int height)
{
    mBmp.Save(width, height);
    mBmp.Close();
}
