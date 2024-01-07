#ifndef _LIBCAMCOREIF_H_
#define _LIBCAMCOREIF_H_

#include <mutex>
#include "libcamdef.h"

namespace NSCAM {
class ICamCore {
public:
    static  ErrorCode GetCameraCount    (unsigned int*);
    static  ICamCore* CreateInstance    (unsigned int);

    virtual ErrorCode StartPreview      (RenderCallback)                        = 0;
    virtual ErrorCode StopPreview       (void)                                  = 0;
    virtual ErrorCode TakePhoto         (const char*)                           = 0;
    virtual ErrorCode StartRecord       (const char*)                           = 0;
    virtual ErrorCode StopRecord        (void)                                  = 0;
    virtual ErrorCode GetSupportedFormat(PinType, MediaFormat*, unsigned int*)  = 0;
    virtual ErrorCode GetCurrentFormat  (PinType, MediaFormat*)                 = 0;
    virtual ErrorCode SetCurrentFormat  (PinType, unsigned int)                 = 0;
};
}

#endif