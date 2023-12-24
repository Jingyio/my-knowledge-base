#pragma once

#include <Windows.h>
#include <wdf.h>

NTSTATUS
QueueCreate(
    _In_ WDFDEVICE  Device
);