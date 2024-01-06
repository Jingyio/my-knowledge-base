#include "queue.h"
#include "driver.h"
#include "exports.h"

VOID
DeviceControl(
	_In_ WDFQUEUE   Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t     OutputBufferLength,
	_In_ size_t     InputBufferLength,
	_In_ ULONG      IoControlCode
)
{
	NTSTATUS status = STATUS_SUCCESS;
	char matchingPatten[] = "Hi";
	char response[] = "Good Boy~~";
	PVOID ptr = 0;

	UNREFERENCED_PARAMETER(Queue);

	switch (IoControlCode)
	{
	case IOCTL_SAYHELLO:
		if (InputBufferLength <= strlen(matchingPatten) || OutputBufferLength <= strlen(response))
		{
			KdPrintEx((DPFLTR_IHVDRIVER_ID,
				DPFLTR_ERROR_LEVEL,
				"[%s] Check Parameters Failed, Require Input Size: %d, Output Size: %d, Actual Input Size: %d, Output Size: %d\n",
				strlen(matchingPatten),
				strlen(response),
				InputBufferLength,
				OutputBufferLength,
				MOD));
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
			break;
		}

		status = WdfRequestRetrieveInputBuffer(Request, strlen(matchingPatten), &ptr, NULL);
		if (!NT_SUCCESS(status))
		{
			KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] WdfRequestRetrieveInputBuffer Failed\n", MOD));
			WdfRequestComplete(Request, STATUS_UNSUCCESSFUL);
			break;
		}

		if (!strcmp(matchingPatten, (const char*)ptr))
		{
			status = WdfRequestRetrieveOutputBuffer(Request, strlen(response), &ptr, NULL);
			if (NT_SUCCESS(status))
			{
				strcpy((char*)ptr, response);
				WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, strlen(response));
			}
		}
		else
		{
			WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
		}
		break;
	default:
		WdfRequestComplete(Request, STATUS_INVALID_PARAMETER);
		break;
	}
}

NTSTATUS
QueueCreate(
	_In_ WDFDEVICE  Device
)
{
	WDFQUEUE queue = { 0 };
	NTSTATUS status = STATUS_SUCCESS;
	WDF_IO_QUEUE_CONFIG queueConfig = { 0 };

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);
	queueConfig.EvtIoDeviceControl = DeviceControl;

	status = WdfIoQueueCreate(Device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
	if (!NT_SUCCESS(status))
		KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "[%s] WdfIoQueueCreate Failed\n", MOD));

	return status;
}