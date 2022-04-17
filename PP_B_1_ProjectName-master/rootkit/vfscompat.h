#pragma once

NTSTATUS VFSCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);
NTSTATUS VFSIRPUnsupported(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);
