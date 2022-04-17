#include <ntddk.h>

#include "vfs.h"

NTSTATUS 
VFSCreateClose(
        IN PDEVICE_OBJECT       DeviceObject,
        IN PIRP                 irp
       	)

/*++

  Routine Description:

  	Handler for IRP_MJ_(CREATE|CLOSE) we just complete these requests 
	as being successful since they are not really applicable with what
	our driver does
    
  Arguments:
  
  	DeviceObject - DeviceObject associated with device IRP is destined 

	irp - I/O Request Packet associated with request

  Return Value:
  
  	Always returns successful status

--*/

{
	UNREFERENCED_PARAMETER(DeviceObject);

        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = 0;

        IoCompleteRequest(irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
}

NTSTATUS 
VFSIRPUnsupported(
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
