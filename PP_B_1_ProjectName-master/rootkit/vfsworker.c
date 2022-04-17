#include <ntddk.h>

#include "vfs.h"
#include "vfsio.h"

NTSTATUS 
PrepareVFSWorkerThread(
		IN PDEVICE_OBJECT DeviceObject
		) 

/*++

Routine Description:

    Creates a worker thread which is responsible for handling read/write operations
    to the virtual filesystem

Arguments:

    TODO:

Return Value:

    TODO:

--*/

{
	HANDLE WorkerThread; 
	NTSTATUS status;
	PVFS_DEVICE_EXTENSION VFSExtension;

	VFSExtension = VFS_EXTENSION(DeviceObject);

	//
	// Initialize synchronization primitives used for synchronizing
	// access to/form the virtual filesystem
	//

	InitializeListHead(&VFSExtension->QueueListHead);
	KeInitializeSpinLock(&VFSExtension->lockQueue);                                      
	KeInitializeSemaphore(&VFSExtension->semQueue, 0 , MAXLONG); 

	//
	// Create worker thread which will be responsible for handling read/write
	// requests to the virtual filesystem
	//

	status = PsCreateSystemThread(&WorkerThread,
			                (ACCESS_MASK)0, 
					NULL, 
					(HANDLE)0, 
			                NULL, 
					VFSWorkerThread, 
			                DeviceObject);

	if(!NT_SUCCESS(status)) {
		DbgPrint("[VFS] Failed to Create Worker Thread");
		return status;
	}

	ZwClose(WorkerThread);

	return status;
}

VOID 
VFSWorkerThread(
		IN PDEVICE_OBJECT DeviceObject
		) 

/*++

Routine Description:

    Waits for read/write operations to be added to work queue and waits in
    an alertable state for items to be added to the queue if it is empty

Arguments:

    TODO:

Return Value:

    VOID (no return)

--*/

{
	PVFS_DEVICE_EXTENSION VFSExtension = VFS_EXTENSION(DeviceObject);
	PLIST_ENTRY ListEntry;
	PIRP irp;
	PIO_STACK_LOCATION irpStack;

	PAGED_CODE();

	while(TRUE) {

		//
		// If we don't have any more requests to process we just sleep and 
		// for more requets to be added to the queue
		//

		KeWaitForSingleObject(&VFSExtension->semQueue, 
				      Executive, 
				      KernelMode, 
				      FALSE, 
				      NULL);

		ListEntry = ExInterlockedRemoveHeadList(&VFSExtension->QueueListHead,
				                        &VFSExtension->lockQueue);
		irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);

		//
		// Route request to read/write handler
		//

		irpStack = IoGetCurrentIrpStackLocation(irp);

		if(irpStack->MajorFunction == IRP_MJ_READ) {
			VFSRead(DeviceObject, irp);
		} else if(irpStack->MajorFunction == IRP_MJ_WRITE) {
			VFSWrite(DeviceObject, irp);
		}

	}
}

NTSTATUS
VFSQueueWorkItem(
		IN PDEVICE_OBJECT DeviceObject, 
		IN PIRP irp
		)

/*++

  Routine Description:

	Queues read/write I/O request packet to work queue which will then
	be read by the worker thread which monitors the work queue and
	completes the IRP

  Arguments:

  	DeviceObject - DeviceObject associated with the VFS which is
		       trying to be read/written to
	
	irp - I/O Request Packet associated with the read/write request

  Return Value:
  	
  	Returns STATUS_PENDING to indicate that the IRP is waiting to 
	be processed by worker thread

--*/

{
	PVFS_DEVICE_EXTENSION VFSExtension = VFS_EXTENSION(DeviceObject);

	IoMarkIrpPending(irp);

	ExInterlockedInsertTailList(&VFSExtension->QueueListHead,
			&irp->Tail.Overlay.ListEntry,
			&VFSExtension->lockQueue);

	KeReleaseSemaphore(&VFSExtension->semQueue, 0, 1, FALSE);

	return STATUS_PENDING;
}
