#pragma once

#include <ntddk.h>
#include <wdf.h>

NTSTATUS
QueueCreate(
    _In_ WDFDEVICE  Device
);