#include "driver.h"
#include "device.h"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD DeviceAdd;
EVT_WDF_DRIVER_UNLOAD DriverUnload;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{
    WDF_DRIVER_CONFIG config = { 0 };
    NTSTATUS status = STATUS_SUCCESS;

    WDF_DRIVER_CONFIG_INIT(&config, DeviceAdd);
    config.EvtDriverUnload = DriverUnload;

    status = WdfDriverCreate(DriverObject, RegistryPath, NULL, &config, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status))
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] WdfDriverCreate Failed\n", MOD));

    return status;
}

VOID
DriverUnload(
    _In_    WDFDRIVER       Driver
)
{
    UNREFERENCED_PARAMETER(Driver);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] Driver Unload\n", MOD));
}

NTSTATUS
DeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Driver);

    status = DeviceCreate(DeviceInit);
    if (!NT_SUCCESS(status))
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] DeviceCreate Failed\n", MOD));

    return status;
}