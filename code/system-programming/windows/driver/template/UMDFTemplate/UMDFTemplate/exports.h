#pragma once

#include <initguid.h>

DEFINE_GUID(GUID_DEVINTERFACE_UMDFTEMPLATE,
    0xb2ca070f, 0x6ce1, 0xd3be, 0xba, 0x58, 0x6d, 0x80, 0x0a, 0xbb, 0x8e, 0xa6);

#define IOCTL_SAYHELLO CTL_CODE(FILE_DEVICE_UNKNOWN, 0x200, METHOD_BUFFERED, FILE_ANY_ACCESS)
