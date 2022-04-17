#pragma once

NTSTATUS
VFSQueueWorkItem(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp
		);

NTSTATUS 
PrepareVFSWorkerThread(
		IN PDEVICE_OBJECT DeviceObject
		);

VOID 
VFSWorkerThread(
		IN PDEVICE_OBJECT DeviceObject
		);
