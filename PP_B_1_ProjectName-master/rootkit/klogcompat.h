#pragma once

#include "keylog.h"

NTSTATUS 
DispatchIRPUnsupported(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
	);

NTSTATUS 
DispatchPassThrough(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp
		);
