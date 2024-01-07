#ifndef _LIBFORMATCONVERTER_H_
#define _LIBFORMATCONVERTER_H_

#include "libcamdef.h"

ErrorCode YUY2toRGBA(unsigned char*, unsigned int, unsigned int, long, unsigned char*);
ErrorCode NV12toRGBA(unsigned char*, unsigned int, unsigned int, long, unsigned char*);
#endif