#include <ntddk.h>

#include "vfs.h"

NTSTATUS
VFSRead(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp
    )

/*++
  
  Routine Description:
  
  	Handler for IRP_MJ_READ requests to VFS
	
  Arguments:
  
  	DeviceObject - DeviceObject associated with device IRP is destined 
	
	irp - I/O Request Packet associated with request

Return Value:

    	Always returns successfully

--*/

{
	PUCHAR src;
	PUCHAR dest;
	PIO_STACK_LOCATION irpStack;
	PVFS_DEVICE_EXTENSION VFSExtension = VFS_EXTENSION(DeviceObject);

	irpStack = IoGetCurrentIrpStackLocation(irp);

        src = (PUCHAR)(VFSExtension->DiskImage + irpStack->Parameters.Read.ByteOffset.LowPart);
	dest = MmGetSystemAddressForMdl(irp->MdlAddress);

        RtlCopyBytes(dest, src, irpStack->Parameters.Read.Length);

	irp->IoStatus.Information = irpStack->Parameters.Read.Length;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS
VFSWrite(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
    )

/*++

  Routine Description:
  
  	Handler for IRP_MJ_WRITE requests to VFS 
	
  Arguments:

  	DeviceObject - device associated with the IRP
	
	irp - I/O Request Packet

Return Value:

	Always returns successfully

--*/

{
	PUCHAR src;
	PUCHAR dest;
	PIO_STACK_LOCATION irpStack;
	PVFS_DEVICE_EXTENSION VFSExtension = VFS_EXTENSION(DeviceObject);
	IO_STATUS_BLOCK fuck;

	irpStack = IoGetCurrentIrpStackLocation(irp);

        src = (PUCHAR)(VFSExtension->DiskImage + irpStack->Parameters.Write.ByteOffset.LowPart);
	dest = MmGetSystemAddressForMdl(irp->MdlAddress);

        RtlCopyBytes(src, dest, irpStack->Parameters.Write.Length);

	irp->IoStatus.Information = irpStack->Parameters.Read.Length;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}
