#pragma once

#include <ntddk.h>
#include <wdf.h>

NTSTATUS
DeviceCreate(
    _Inout_ PWDFDEVICE_INIT DeviceInit
);