#pragma once

#include <Windows.h>
#include <wdf.h>
#include <initguid.h>

NTSTATUS
DeviceCreate(
    _Inout_ PWDFDEVICE_INIT DeviceInit
);