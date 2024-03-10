#ifndef _LIBFORMATCONVERTER_H_
#define _LIBFORMATCONVERTER_H_

#include "libcamdef.h"

ErrorCode YUY2toRGBA(unsigned char*, unsigned long long, unsigned long long, long long, unsigned char*);
ErrorCode NV12toRGBA(unsigned char*, unsigned long long, unsigned long long, long long, unsigned char*);
#endif