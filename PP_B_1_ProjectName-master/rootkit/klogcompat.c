#include <ntddk.h>

#include "klogcompat.h"
#include "keylog.h"

NTSTATUS 
DispatchIRPUnsupported(
	IN PDEVICE_OBJECT DeviceObject, 
	IN PIRP irp
	)

/*++

  Routine Description:
  
  	Generic handler for IRPs which are not supported we return a
	status of STATUS_NOT_IMPLEMENTED

  Arguments:

  	DeviceObject - device associated with the IRP
	
	irp - I/O Request Packet

  Return Value:
  
  	Returns status saying that the request is not implemented

--*/

{
	NTSTATUS status;

	//
	// Since we are the lowest level driver in the device stack we
	// do not have a lower level driver for which we need to pass
	// unsupported IRPs 
	//

	status = irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

NTSTATUS 
DispatchPassThrough(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp
		)

/*++

  Routine Description:

  	TODO:

  Arguments:
  
  	TODO:

  Return Value:
  
  	TODO:

--*/

{
	IoSkipCurrentIrpStackLocation(irp);
	return IoCallDriver(((PKEYLOG_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->KeyboardDevice ,irp);
}
