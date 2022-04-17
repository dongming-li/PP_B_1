#pragma once

#include <ntddk.h>

#include "vfs.h"

//
// IRP Major Function for Handling IOCTL
//

NTSTATUS VFSIoctl(IN PDEVICE_OBJECT DeviceObject, IN PIRP irp);
