#pragma once

NTSTATUS VFSRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);
NTSTATUS VFSWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);
