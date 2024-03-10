#ifndef _LIBSINKDEVICE_H_
#define _LIBSINKDEVICE_H_

#include "libmfsource.h"

class ISinkDevice
{
public:
    explicit ISinkDevice   (void) = default;
    virtual ~ISinkDevice  (void) = default;
    virtual HRESULT OnDataCallback(DWORD, DWORD, IMFSample*, ULONGLONG) = 0;
    virtual unsigned int GetStreamIndex(void) = 0;
    virtual bool IsOneShot (void) = 0;
};

#endif