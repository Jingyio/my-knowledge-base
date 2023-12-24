#include "device.h"
#include "driver.h"
#include "queue.h"
#include "exports.h"

NTSTATUS
DeviceCreate(
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    WDFDEVICE device = { 0 };
    NTSTATUS status = STATUS_SUCCESS;

    status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &device);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] WdfDeviceCreate Failed\n", MOD));
        return status;
    }

    status = WdfDeviceCreateDeviceInterface(device, &GUID_DEVINTERFACE_KMDFTEMPLATE, NULL);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] WdfDeviceCreateDeviceInterface Failed\n", MOD));
        return status;
    }

    status = QueueCreate(device);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] QueueCreate Failed\n", MOD));
        return status;
    }

    return status;
}